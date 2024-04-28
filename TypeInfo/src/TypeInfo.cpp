#include "TypeInfo.h"

#include "Inherit.h"

#include <fstream>

namespace re::typeinfo {
namespace detail {
// Variable for keeping track of the next type's index.
// This also acts as a type count variable.
TypeIndex globalTypeIndex = 0;

// Declare type info for void, which is used as "none" type.
class VoidSetupHelper
{
	static SetupHelper<void, void> voidSetupHelper;
public:
	static const TypeInfo* getTypeInfo()
	{
		return voidSetupHelper.getTypeInfo();
	}
};

SetupHelper<void, void> VoidSetupHelper::voidSetupHelper;

// Wrap this into a function so that the vector isn't initialized during CRT.
static std::vector<std::shared_ptr<TypeInfo>>& getTypeInfoDatabase()
{
	static std::vector<std::shared_ptr<TypeInfo>> typeInfoDatabase(UINT16_MAX, nullptr);
	
	return typeInfoDatabase;
}

// Allocates a shared pointer to type info then adds it to the type info list.
std::shared_ptr<TypeInfo> createTypeInfo(const std::string& className, uint32_t typeSize, TypeIndex typeIndex, TypeIndex parentTypeIndex)
{
	auto& typeInfoDB = getTypeInfoDatabase();
	typeInfoDB[typeIndex] = std::make_shared<TypeInfo>(className, typeSize, typeIndex, parentTypeIndex);

	return typeInfoDB[typeIndex];
}

std::string fixupTypeName(const char* typeName)
{
	std::string typeNameStr = typeName;

	size_t scopeSep = typeNameStr.find("::");
	while (scopeSep != std::string::npos)
	{
		typeNameStr = typeNameStr.replace(scopeSep, 2, ".");
		scopeSep = typeNameStr.find("::");
	}

	return typeNameStr;
}
}

// Fill in extra data after all types have been registered.
void initialize()
{
	auto& typeInfoDB = detail::getTypeInfoDatabase();

	// Remove excess reserved types.
	typeInfoDB.resize(detail::globalTypeIndex);
	typeInfoDB.shrink_to_fit();

	// Resolve type's parents and set the pointer to them.
	// Types are added to parent's list of child types.
	for (auto& typeInfo : typeInfoDB)
	{
		// Get parent type in the database from parent type index.
		std::shared_ptr<TypeInfo> parentType = typeInfoDB[typeInfo->parentTypeIndex];

		// Avoid parenting to self.
		if (parentType && parentType->typeIndex != typeInfo->typeIndex)
		{
			// Set pointer to the parent type.
			typeInfo->parentType = parentType;

			// Add this type to the parent's list of child types.
			parentType->childTypes.push_back(typeInfo);
		}
		else
			typeInfo->parentType = nullptr;
	}
}

// void is used as "none" type.
const TypeInfo* getNoneType()
{
	return detail::getTypeInfo<void, void>().get();
}

bool isNoneType(const TypeInfo* typeInfo)
{
	return false;
}

#if _DEBUG
// Recursive function for writing type hierarchy to.
void dbgPrintType(std::ostream& outputStream, const TypeInfo* typeInfo, int depth)
{
	for (int i = 0; i < depth; i++)
		outputStream << '\t';

	outputStream << typeInfo->className << ": " << std::hex << typeInfo->typeSize << "\n";

	for (const auto& child : typeInfo->childTypes)
	{
		dbgPrintType(outputStream, child.get(), depth + 1);
	}
}

// Outputs the type hierarchy to a text file.
void dbgWriteTypeTreeToFile(std::ostream& outputStream)
{
	const TypeInfo* rootPtr = getNoneType();

	dbgPrintType(outputStream, rootPtr, 0);
}

// Returns a string showing the hierarchy tree of a type.
// None -> Object -> MyClass
std::string dbgGetTypeHierarchy(const TypeInfo* typeInfo)
{
	std::string result;

	while (typeInfo)
	{
		result.insert(0, typeInfo->className);
		
		typeInfo = typeInfo->parentType.get();

		if (typeInfo)
			result.insert(0, " -> ");
	}

	return result;
}
#endif

}