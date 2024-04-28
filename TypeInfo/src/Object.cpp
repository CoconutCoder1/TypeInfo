#include "Object.h"

namespace re {

typeinfo::detail::SetupHelper<void, Object> Object::typeInfoSetupHelper;

const TypeInfo* Object::getTypeInfo() const
{
	return typeInfoSetupHelper.getTypeInfo();
}

}