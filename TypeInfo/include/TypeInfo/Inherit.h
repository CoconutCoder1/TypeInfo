#pragma once

#include "TypeInfo.h"

namespace re {

// This class should be used unless we want to declare type info manually.
template<class Base, class Derived>
class Inherit : public Base
{
	static typeinfo::detail::SetupHelper<Base, Derived> typeInfoSetupHelper;
public:
	using ParentType = Base;

	Inherit()
	{
	}

	virtual ~Inherit() {}

	virtual const TypeInfo* getTypeInfo() const override
	{
		return typeInfoSetupHelper.getTypeInfo();
	}
};

template<class Base, class Derived>
typeinfo::detail::SetupHelper<Base, Derived> Inherit<Base, Derived>::typeInfoSetupHelper;

}