#pragma once

#include <string>
#include <vector>
#include <memory>

namespace re {

using TypeIndex = uint16_t;

struct TypeInfo
{
	TypeInfo(const std::string& className, uint32_t typeSize, TypeIndex typeIndex, TypeIndex parentTypeIndex)
		: className(className), typeSize(typeSize), typeIndex(typeIndex), parentTypeIndex(parentTypeIndex)
	{
	}

	const std::string className;
	const uint32_t typeSize;
	const TypeIndex typeIndex;
	const TypeIndex parentTypeIndex;
	std::shared_ptr<TypeInfo> parentType;
	std::vector<std::shared_ptr<TypeInfo>> childTypes;
};

namespace typeinfo {

void initialize();

#if _DEBUG
void dbgWriteTypeTreeToFile(std::ostream& outputStream);
std::string dbgGetTypeHierarchy(const TypeInfo* typeInfo);
#endif

namespace detail {

extern TypeIndex globalTypeIndex;
std::shared_ptr<TypeInfo> createTypeInfo(const std::string& className, uint32_t typeSize, TypeIndex typeIndex, TypeIndex parentTypeIndex);

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
			getTypeIndex<Base>()
		);

		return typeInfo;
	}
	else
	{
		static std::shared_ptr<TypeInfo> typeInfo = createTypeInfo(
			getTypeName<Derived>(),
			sizeof(Derived),
			getTypeIndex<Derived>(),
			getTypeIndex<Base>()
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

// All types inherit from the "none" type if not from Object.
const TypeInfo* getNoneType();

// Returns true if the specified type info pointer is the "none" type.
bool isNoneType(const TypeInfo* typeInfo);

}
}