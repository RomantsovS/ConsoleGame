#pragma hdrstop
#include "idlib/precompiled.h"

#include "tr_local.h"

/*
===================
R_FreeEntityDefDerivedData

Used by both FreeEntityDef and UpdateEntityDef
Does not actually free the entityDef.
===================
*/
void R_FreeEntityDefDerivedData(idRenderEntityLocal* def, bool keepDecals, bool keepCachedDynamicModel) noexcept {
	// free the entityRefs from the areas
	for (auto ref = def->entityRefs.get(); ref;) {
		auto next = ref->ownerNext.get();

		// unlink from the area
		ref->areaNext->areaPrev = ref->areaPrev;
		ref->areaPrev->areaNext = ref->areaNext;

		// put it back on the free list for reuse
		//def->world->areaReferenceAllocator.Free(ref);

		ref = next;
	}
	def->entityRefs = nullptr;
}

/*
===============
R_CreateEntityRefs

Creates all needed model references in portal areas,
chaining them to both the area and the entityDef.

Bumps tr.viewCount, which means viewCount can change many times each frame.
===============
*/
void R_CreateEntityRefs(idRenderEntityLocal* entity) {
	if (!entity->parms.hModel) {
		entity->parms.hModel = renderModelManager->DefaultModel();
	}

	// if the entity hasn't been fully specified due to expensive animation calcs
	// for md5 and particles, use the provided conservative bounds.
	/*if (entity->parms.callback != NULL) {
		entity->localReferenceBounds = entity->parms.bounds;
	}
	else {*/
		//entity->localReferenceBounds = entity->parms.hModel->Bounds(&entity->parms);
	//}

	// bump the view count so we can tell if an
	// area already has a reference
	tr.viewCount++;

	// push the model frustum down the BSP tree into areas
	entity->world->PushFrustumIntoTree(entity);
}

/*
===================
R_FreeDerivedData

ReloadModels and RegenerateWorld call this
===================
*/
void R_FreeDerivedData() noexcept {
	for(auto &rw : tr.worlds) {
		for (auto &def : rw->entityDefs) {
			if (!def) {
				continue;
			}
			R_FreeEntityDefDerivedData(def.get(), false, false);
		}
	}
}

/*
===================
R_CheckForEntityDefsUsingModel
===================
*/
void R_CheckForEntityDefsUsingModel(std::shared_ptr<idRenderModel> model) noexcept {
	for (auto &rw : tr.worlds)
	{
		for (auto &def : rw->entityDefs)
		{
			if (!def) {
				continue;
			}
			if (def->parms.hModel == model) {
				//assert( 0 );
				// this should never happen but Radiant messes it up all the time so just free the derived data
				R_FreeEntityDefDerivedData(def.get(), false, false);
			}
		}
	}
}