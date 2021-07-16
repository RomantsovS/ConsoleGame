#pragma hdrstop
#include <precompiled.h>

#include "../Game_local.h"

CLASS_DECLARATION(idActor, idAI)
END_CLASS

idAI::idAI() {
}

idAI::~idAI() {
}

void idAI::Spawn() {
	fl.takedamage = !spawnArgs.GetBool("noDamage");
}

void idAI::Think() {
}

/*
================
idAI::Hide
================
*/
void idAI::Hide() {
	idActor::Hide();
	fl.takedamage = false;
}

/*
=====================
idAI::Killed
=====================
*/
void idAI::Killed(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker, int damage,
	const Vector2& dir) {

	//const std::string modelDeath;

	/*if (spawnArgs.GetString("model_death", "", &modelDeath)) {
		SetModel(modelDeath);
		physicsObj.SetLinearVelocity(vec3_zero);
		physicsObj.PutToRest();
		physicsObj.DisableImpact();
	}*/
	PostEventMS(&EV_Remove, 0);
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

void AISimple::Hide() {
	idAI::Hide();

	//physicsObj.SetContents(0);
	physicsObj->GetClipModel()->Unlink();
}

void AISimple::Killed(std::shared_ptr<idEntity> inflictor, std::shared_ptr<idEntity> attacker, int damage,
	const Vector2& dir) {

	// make monster nonsolid
	//physicsObj.SetContents(0);
	physicsObj->GetClipModel()->Unlink();

	idAI::Killed(inflictor, attacker, damage, dir);
}
