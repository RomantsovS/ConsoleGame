#include "tr_local.h"

int viewEntity_t::count = 0;

idRenderEntity::~idRenderEntity()
{
}

idRenderEntityLocal::idRenderEntityLocal()
{
	parms.hModel = nullptr;

	world = nullptr;
	index = 0;
	viewEntity = nullptr;
	entityRefs = nullptr;
}

idRenderEntityLocal::~idRenderEntityLocal()
{
}
