#include <TypeInfo/Object.h>
#include <TypeInfo/TypeInfo.h>

#include <iostream>

#pragma comment(lib, "TypeInfo.lib")

#define TEST_TYPEINFO(_type, _expectedName, _expectedSize) \
{ \
	const re::TypeInfo* typeInfoPtr = re::typeinfo::getTypeInfo<_type>();	\
	if (typeInfoPtr->typeSize != _expectedSize) \
		std::cerr \
			<< "TypeInfo test failed: expected size 0x" << std::hex << _expectedSize \
			<< " but got 0x" << std::hex << typeInfoPtr->typeSize << "\n"; \
	else if (typeInfoPtr->className != _expectedName) \
		std::cerr \
			<< "TypeInfo test failed: expected name \"" << _expectedName \
			<< "\" but got \"" << typeInfoPtr->className << "\"\n"; \
	else \
		std::cout << "TypeInfo test success \"" << _expectedName << "\"\n"; \
}

class MyClass : public re::Inherit<re::Object, MyClass>
{
};

namespace mynamespace {
class MyClass : public re::Inherit<re::Object, MyClass>
{
};

namespace mynamespace2 {
class MyClass : public re::Inherit<re::Object, MyClass>
{
};
}
}

int main()
{
	// Type in global scope
	TEST_TYPEINFO(MyClass, "MyClass", sizeof(MyClass));

	// Type within namespace
	TEST_TYPEINFO(mynamespace::MyClass, "mynamespace.MyClass", sizeof(mynamespace::MyClass));

	// Type within multiple namespaces
	TEST_TYPEINFO(
		mynamespace::mynamespace2::MyClass,
		"mynamespace.mynamespace2.MyClass",
		sizeof(mynamespace::mynamespace2::MyClass)
	);

	return 0;
}