#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace re {

using TypeIndex = uint16_t;

class TypeInfo;
class Object;

namespace typeinfo {

void initialize();

#if _DEBUG
void dbgWriteTypeTreeToFile(std::ostream& outputStream);
std::string dbgGetTypeHierarchy(const TypeInfo* typeInfo);
#endif

using CreateFn = std::function<std::shared_ptr<Object>()>;

namespace detail {

extern TypeIndex globalTypeIndex;
std::shared_ptr<TypeInfo> createTypeInfo(const std::string& className, uint32_t typeSize, TypeIndex typeIndex, TypeIndex parentTypeIndex, CreateFn createFn);

template<class T>
TypeIndex getTypeIndex()
{
	// Assign a type index to this type once
	// and then increment the global type index.
	static TypeIndex typeIndex = globalTypeIndex++;

	return typeIndex;
}

std::string fixupTypeName(const char* typeName);

template<typename T>
std::string getTypeName()
{
	const char* typeName = typeid(T).name();
	
	if (typeName[0] == 'c') // class MyClass
		typeName += 6;
	else if (typeName[0] == 's') // struct MyStruct
		typeName += 7;

	return fixupTypeName(typeName);
}

template<class T>
std::shared_ptr<Object> createTypeProxy()
{
	requires { std::is_base_of_v<Object, T>; };

	if constexpr (std::is_abstract_v<T> || !std::is_default_constructible_v<T>)
	{
		return nullptr;
	}
	else
	{
		return std::make_shared<T>();
	}
}

template<class Base, class Derived>
std::shared_ptr<TypeInfo> getTypeInfo()
{
	// void is considered "none" type.
	if constexpr (std::is_void<Derived>())
	{
		static std::shared_ptr<TypeInfo> typeInfo = createTypeInfo(
			"None",
			0,
			getTypeIndex<Derived>(),
			getTypeIndex<Base>(),
			nullptr
		);

		return typeInfo;
	}
	else
	{
		static std::shared_ptr<TypeInfo> typeInfo = createTypeInfo(
			getTypeName<Derived>(),
			sizeof(Derived),
			getTypeIndex<Derived>(),
			getTypeIndex<Base>(),
			&createTypeProxy<Derived>
		);

		return typeInfo;
	}
}

template<class Base, class Derived>
class SetupHelper
{
public:
	const TypeInfo* getTypeInfo() const
	{
		return typeInfoSetupVar.get();
	}

protected:
	static std::shared_ptr<TypeInfo> typeInfoSetupVar;
};

template<class Base, class Derived>
std::shared_ptr<TypeInfo> SetupHelper<Base, Derived>::typeInfoSetupVar = detail::getTypeInfo<Base, Derived>();

}

// This requires that the type uses Inherit<>
// or has its own "ParentType" defined.
template<class Type>
const TypeInfo* getTypeInfo()
{
	requires { typename Type::ParentType; };
	return detail::getTypeInfo<Type::ParentType, Type>().get();
}

template<class Type>
TypeIndex getTypeIndex()
{
	return getTypeInfo<Type>()->typeIndex;
}

// Get type info from its type index.
const TypeInfo* getTypeByIndex(TypeIndex typeIndex);

// All types inherit from the "none" type if not from Object.
const TypeInfo* getNoneType();

// Returns true if the specified type info pointer is the "none" type.
bool isNoneType(const TypeInfo* typeInfo);

}

// TypeInfo implementation.
class TypeInfo
{
public:
	TypeInfo(const std::string& className, uint32_t typeSize, TypeIndex typeIndex, TypeIndex parentTypeIndex, typeinfo::CreateFn createFn)
		: className(className), typeSize(typeSize), typeIndex(typeIndex), parentTypeIndex(parentTypeIndex), createFn(createFn)
	{
	}

	const std::string className;
	const uint32_t typeSize;
	const TypeIndex typeIndex;
	const TypeIndex parentTypeIndex;
	const typeinfo::CreateFn createFn;
	std::shared_ptr<TypeInfo> parentType;
	std::vector<std::shared_ptr<TypeInfo>> childTypes;

	bool isBaseOf(TypeIndex typeIndex) const;
	bool isNoneType() const;

	template<class Type>
	bool isBaseOf() const
	{
		return isBaseOf(typeinfo::getTypeInfo<Type>()->typeIndex);
	}

	template<class FromType>
	bool isDerived() const
	{
		return isBaseOf<FromType>();
	}
};
}