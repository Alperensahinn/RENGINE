#include "RObject.h"

Ruya::RObject::RObject(RObjectType type) : rObjectType(type)
{
}

Ruya::RObject::~RObject()
{
}

Ruya::RObjectType Ruya::RObject::GetRObjectType()
{
	return rObjectType;
}
