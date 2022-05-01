#pragma hdrstop
#include "idlib/precompiled.h"

#include "tr_local.h"

/*
=============
R_SetEntityDefViewEntity

If the entityDef is not already on the viewEntity list, create
a viewEntity and add it to the list with an empty scissor rect.
=============
*/
std::shared_ptr<viewEntity_t> R_SetEntityDefViewEntity(idRenderEntityLocal* def) {
	if (def->viewCount == tr.viewCount) {
		// already set up for this frame
		return def->viewEntity;
	}
	def->viewCount = tr.viewCount;

	auto vModel = std::make_shared<viewEntity_t>();
	vModel->entityDef = def;

	vModel->next = tr.viewDef->viewEntitys;
	tr.viewDef->viewEntitys = vModel;

	def->viewEntity = vModel;

	return vModel;
}

/*
===================
AddAreaViewEntities

Any models that are visible through the current portalStack will have their
scissor rect updated.
===================
*/
void idRenderWorldLocal::AddAreaViewEntities(int areaNum) {
	portalArea_t* area = &portalAreas[areaNum];

	for (auto ref = area->entityRefs->areaNext; ref != area->entityRefs;
		ref = ref->areaNext) {
		auto entity = ref->entity;

		// cull reference bounds
		/*if (CullEntityByPortals(entity, ps)) {
				// we are culled out through this portal chain, but it might
				// still be visible through others
				continue;
		}*/

		auto vEnt = R_SetEntityDefViewEntity(entity);
	}
}

/*
===================
AddAreaToView

This may be entered multiple times with different planes
if more than one portal sees into the area
===================
*/
void idRenderWorldLocal::AddAreaToView(int areaNum) {
	// mark the viewCount, so r_showPortals can display the considered portals
	portalAreas[areaNum].viewCount = tr.viewCount;

	// add the models and lights, using more precise culling to the planes
	AddAreaViewEntities(areaNum);
}

void idRenderWorldLocal::FindViewLightsAndEntities() {
	// bumping this counter invalidates cached viewLights / viewEntities,
	// when a light or entity is next considered, it will create a new
	// viewLight / viewEntity
	tr.viewCount++;

	tr.viewDef->viewEntitys = nullptr;

	tr.viewDef->areaNum = 0;

	AddAreaToView(tr.viewDef->areaNum);
}