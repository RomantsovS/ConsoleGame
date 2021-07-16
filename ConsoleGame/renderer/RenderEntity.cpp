#pragma hdrstop
#include <precompiled.h>

#include "tr_local.h"

idRenderEntity::~idRenderEntity() {
}

idRenderEntityLocal::idRenderEntityLocal() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idRenderEntityLocal");
#endif // DEBUG_PRINT_Ctor_Dtor

	parms.hModel = nullptr;

	index = 0;
	viewEntity = nullptr;
	entityRefs = nullptr;
}

idRenderEntityLocal::~idRenderEntityLocal() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "idRenderEntityLocal");
#endif // DEBUG_PRINT_Ctor_Dtor
}
