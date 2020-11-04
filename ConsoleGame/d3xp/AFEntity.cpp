#include "AFEntity.h"
#include "Game_local.h"
#include "../renderer/ModelManager.h"

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
	physicsObj = std::make_shared<idPhysics_AF>();
	physicsObj->SetSelf(shared_from_this());
}

/*
================
idMultiModelAF::~idMultiModelAF
================
*/
idMultiModelAF::~idMultiModelAF() {
	for (size_t i = 0; i < modelDefHandles.size(); i++) {
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
void idMultiModelAF::SetModelForId(int id, const std::string& modelName) {
	modelHandles.resize(id + 1);
	modelDefHandles.resize(id + 1, -1);
	modelHandles[id] = renderModelManager->FindModel(modelName);
}

/*
================
idMultiModelAF::Present
================
*/
void idMultiModelAF::Present() {
	// don't present to the renderer if the entity hasn't changed
	if (!(thinkFlags & TH_UPDATEVISUALS)) {
		return;
	}
	BecomeInactive(TH_UPDATEVISUALS);

	for (size_t i = 0; i < modelHandles.size(); i++) {

		if (!modelHandles[i]) {
			continue;
		}

		renderEntity.origin = physicsObj->GetOrigin(i);
		renderEntity.axis = physicsObj->GetAxis(i);
		renderEntity.hModel = modelHandles[i];

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

/*
===============================================================================

  idChain

===============================================================================
*/

CLASS_DECLARATION(idMultiModelAF, idChain)
END_CLASS

/*
================
idChain::BuildChain

  builds a chain hanging down from the ceiling
  the highest link is a child of the link below it etc.
  this allows an object to be attached to multiple chains while keeping a single tree structure
================
*/
void idChain::BuildChain(const std::string& name, const Vector2& origin, float linkLength, int numLinks) {
	int i;
	std::shared_ptr<idAFBody> body, lastBody;
	Vector2 org;

	idTraceModel trm;
	float density;
	std::string clipModelName;

	// check if a clip model is set
	spawnArgs.GetString("clipmodel", "", &clipModelName);
	if (!clipModelName[0]) {
		clipModelName = spawnArgs.GetString("model");		// use the visual model
	}

	if (!collisionModelManager->TrmFromModel(clipModelName, trm)) {
		gameLocal.Error("idSimpleObject '%s': cannot load collision model %s", name, clipModelName);
		return;
	}

	org = origin;

	for (i = 0; i < numLinks; i++) {
		// add body
		auto clip = std::make_shared<idClipModel>(trm);
		//clip->SetContents(CONTENTS_SOLID);
		clip->Link(gameLocal.clip, shared_from_this(), i, org);
		body = std::make_shared<idAFBody>(name + std::to_string(i), clip, density);
		physicsObj->AddBody(body);

		// visual model for body
		SetModelForId(physicsObj->GetBodyId(body), spawnArgs.GetString("model"));

		org[1] -= linkLength;

		lastBody = body;
	}
}

/*
================
idChain::Spawn
================
*/
void idChain::Spawn() {
	int numLinks;
	Vector2 origin;

	spawnArgs.GetInt("links", "3", numLinks);
	origin = GetPhysics()->GetOrigin();

	// initialize physics
	physicsObj->SetSelf(shared_from_this());
	physicsObj->SetClipMask(MASK_SOLID);
	SetPhysics(physicsObj);

	BuildChain("link", origin, 1.0f, numLinks);

	Vector2 linearVelocity;
	spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
	physicsObj->SetLinearVelocity(linearVelocity, 0);
}