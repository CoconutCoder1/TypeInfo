#pragma once

#include "TypeInfo.h"
#include "Inherit.h"

namespace re {

// All types that wants type info should be derived from this.
class Object
{
	static typeinfo::detail::SetupHelper<void, Object> typeInfoSetupHelper;

public:
	using ParentType = void;

	Object()
	{
		int brk = 0;
	}

	virtual ~Object() {}
	virtual const TypeInfo* getTypeInfo() const;
};

}