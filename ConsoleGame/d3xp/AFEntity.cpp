#include "AFEntity.h"
#include "Game_local.h"

/*
===============================================================================

  idMultiModelAF

===============================================================================
*/

CLASS_DECLARATION(idEntity, idMultiModelAF)
END_CLASS

/*
================
idMultiModelAF::Spawn
================
*/
void idMultiModelAF::Spawn() {
	physicsObj.SetSelf(this);
}

/*
================
idMultiModelAF::~idMultiModelAF
================
*/
idMultiModelAF::~idMultiModelAF() {
	int i;

	for (i = 0; i < modelDefHandles.size(); i++) {
		if (modelDefHandles[i] != -1) {
			gameRenderWorld->FreeEntityDef(modelDefHandles[i]);
			modelDefHandles[i] = -1;
		}
	}
}

/*
================
idMultiModelAF::SetModelForId
================
*/
void idMultiModelAF::SetModelForId(int id, const idStr& modelName) {
	modelHandles.AssureSize(id + 1, NULL);
	modelDefHandles.AssureSize(id + 1, -1);
	modelHandles[id] = renderModelManager->FindModel(modelName);
}

/*
================
idMultiModelAF::Present
================
*/
void idMultiModelAF::Present() {
	int i;

	// don't present to the renderer if the entity hasn't changed
	if (!(thinkFlags & TH_UPDATEVISUALS)) {
		return;
	}
	BecomeInactive(TH_UPDATEVISUALS);

	for (i = 0; i < modelHandles.Num(); i++) {

		if (!modelHandles[i]) {
			continue;
		}

		renderEntity.origin = physicsObj.GetOrigin(i);
		renderEntity.axis = physicsObj.GetAxis(i);
		renderEntity.hModel = modelHandles[i];
		renderEntity.bodyId = i;

		// add to refresh list
		if (modelDefHandles[i] == -1) {
			modelDefHandles[i] = gameRenderWorld->AddEntityDef(&renderEntity);
		}
		else {
			gameRenderWorld->UpdateEntityDef(modelDefHandles[i], &renderEntity);
		}
	}
}

/*
================
idMultiModelAF::Think
================
*/
void idMultiModelAF::Think() {
	RunPhysics();
	Present();
}