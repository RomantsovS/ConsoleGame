#include "idlib/precompiled.h"

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

void idAI::Think() noexcept {
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
void idAI::Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) noexcept {

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
	physicsObj->SetSelf(this);
	//physicsObj->SetClipModel(std::make_shared<idClipModel>(trm), density);
	physicsObj->SetClipModel(std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), density);
	physicsObj->SetOrigin(GetPhysics()->GetOrigin());

	SetPhysics(physicsObj);

	Vector2 linearVelocity;
	spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
	physicsObj->SetLinearVelocity(linearVelocity);

	lastChangeDirection = gameLocal.time;
	directionChangePeriod = spawnArgs.GetInt("directionChangePeriod", "0");
}

void AISimple::Think() noexcept {
	if (directionChangePeriod > 0 && gameLocal.time - lastChangeDirection > directionChangePeriod) {
		auto vel = GetPhysics()->GetLinearVelocity();
		int i = gameLocal.GetRandomValue({-1, 1});
		vel = ((vel.x == 0) ? Vector2(vel.y * i, 0.0f) : Vector2(0.0f, vel.x * i));
		GetPhysics()->SetLinearVelocity(vel);

		lastChangeDirection = gameLocal.time;
	}

	idEntity::Think();
}

void AISimple::Remove() noexcept {
	physicsObj->SetClipModel(nullptr, 0.0f);

	physicsObj = nullptr;
	idEntity::Remove();
}

bool AISimple::Collide(const trace_t& collision, const Vector2& velocity) noexcept {
	auto& other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (collision.c.entityNum == ENTITYNUM_WORLD || other->IsType(AISimple::Type)
			|| other->IsType(idStaticEntity::Type)) {
			Vector2 vel = GetPhysics()->GetLinearVelocity();
			GetPhysics()->SetLinearVelocity(-vel);
		}
		else if (other->IsType(idPlayer::Type)) {
			const std::string& damageDefName = spawnArgs.GetString("def_damage");

			other->Damage(this, this, vec2_origin, damageDefName);
		}
	}

	return false;
}

void AISimple::Hide() {
	idAI::Hide();

	//physicsObj.SetContents(0);
	physicsObj->GetClipModel()->Unlink();
}

void AISimple::Killed(idEntity* inflictor, idEntity* attacker, int damage, const Vector2& dir) noexcept {

	// make monster nonsolid
	//physicsObj.SetContents(0);
	physicsObj->GetClipModel()->Unlink();

	idAI::Killed(inflictor, attacker, damage, dir);
}
