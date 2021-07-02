#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idActor, idAI)
END_CLASS

idAI::idAI() {
}

idAI::~idAI() {
}

void idAI::Spawn() {
}

void idAI::Think() {
}

CLASS_DECLARATION(idAI, AISimple)
END_CLASS

AISimple::AISimple() {
}

AISimple::~AISimple() {
}

void AISimple::Spawn() {
	idTraceModel trm;
	float density;
	std::string clipModelName;

	// check if a clip model is set
	/*spawnArgs.GetString("clipmodel", "", &clipModelName);
	if (!clipModelName[0]) {
		clipModelName = spawnArgs.GetString("model");		// use the visual model
	}

	if (!collisionModelManager->TrmFromModel(clipModelName, trm)) {
		gameLocal.Error("idSimpleObject '%s': cannot load collision model %s", name, clipModelName);
		return;
	}*/

	density = 0.0f;

	physicsObj = std::make_shared<idPhysics_RigidBody>();
	physicsObj->SetSelf(shared_from_this());
	//physicsObj->SetClipModel(std::make_shared<idClipModel>(trm), density);
	physicsObj->SetClipModel(std::make_shared<idClipModel>(GetPhysics()->GetClipModel()), density);
	physicsObj->SetOrigin(GetPhysics()->GetOrigin());

	SetPhysics(physicsObj);

	Vector2 linearVelocity;
	spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
	physicsObj->SetLinearVelocity(linearVelocity);
}

void AISimple::Think() {
	idEntity::Think();
}

void AISimple::Remove() {
	physicsObj->SetClipModel(nullptr, 0.0f);

	physicsObj = nullptr;
	idEntity::Remove();
}

bool AISimple::Collide(const trace_t& collision, const Vector2& velocity) {
	auto other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (collision.c.entityNum == ENTITYNUM_WORLD || other->IsType(AISimple::Type) ||
			other->IsType(idStaticEntity::Type)) {
			auto vel = GetPhysics()->GetLinearVelocity();
			GetPhysics()->SetLinearVelocity(-vel);
		}
	}

	return false;
}
