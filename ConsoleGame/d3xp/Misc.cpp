#include "Misc.h"
#include "Game_local.h"

CLASS_DECLARATION(idEntity, idStaticEntity)
END_CLASS

idStaticEntity::idStaticEntity() {
	//spawnTime = 0;
}

idStaticEntity::~idStaticEntity() {
}

void idStaticEntity::Spawn()
{
	//spawnTime = gameLocal.time;
}

void idStaticEntity::Think() {
	idEntity::Think();
}

CLASS_DECLARATION(idEntity, idSimpleObject)
END_CLASS

idSimpleObject::idSimpleObject() {
}

idSimpleObject::~idSimpleObject() {
}

void idSimpleObject::Spawn() {
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

	density = 0.0f;

	physicsObj = std::make_shared<idPhysics_RigidBody>();
	physicsObj->SetSelf(shared_from_this());
	physicsObj->SetClipModel(std::make_shared<idClipModel>(trm), density);
	physicsObj->SetOrigin(GetPhysics()->GetOrigin());

	SetPhysics(physicsObj);

	Vector2 linearVelocity;
	spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
	physicsObj->SetLinearVelocity(linearVelocity);
}

void idSimpleObject::Think() {
	idEntity::Think();
}

void idSimpleObject::Remove() {
	physicsObj->SetClipModel(nullptr, 0.0f);

	physicsObj = nullptr;
	idEntity::Remove();
}

bool idSimpleObject::Collide(const trace_t& collision, const Vector2& velocity) {
	if (collision.c.entityNum == ENTITYNUM_WORLD) {
		return true;
	}
	else {
		auto ent = gameLocal.entities[collision.c.entityNum];

		if (!ent->IsActive())
			return true;
	}

	return false;
}
