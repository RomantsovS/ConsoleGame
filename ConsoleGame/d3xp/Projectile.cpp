#include <precompiled.h>
#pragma hdrstop

#include "Game_local.h"

const idEventDef EV_Explode("<explode>", "");

CLASS_DECLARATION(idEntity, idProjectile)
	EVENT(EV_Explode, idProjectile::Event_Explode)
END_CLASS

/*
================
idProjectile::idProjectile
================
*/
idProjectile::idProjectile() {
}

/*
================
idProjectile::Spawn
================
*/
void idProjectile::Spawn() {
	physicsObj = std::make_shared<idPhysics_RigidBody>();
	physicsObj->SetSelf(this);
	physicsObj->SetClipModel(std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), 1.0f);
	//physicsObj->SetContents(0);
	physicsObj->SetClipMask(0);
	//physicsObj.PutToRest();
	SetPhysics(physicsObj);
}

/*
================
idProjectile::Create
================
*/
void idProjectile::Create(idEntity* owner, const Vector2& start, const Vector2& dir) {
	idDict args;

	//Unbind();

	physicsObj->SetOrigin(start);
	//physicsObj.SetAxis(axis);

	physicsObj->GetClipModel()->SetOwner(owner);

	this->owner = owner;

	UpdateVisuals();

	state = projectileState_t::CREATED;
}

/*
=================
idProjectile::~idProjectile
=================
*/
idProjectile::~idProjectile() {
}

/*
=================
idProjectile::Launch
=================
*/
void idProjectile::Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity, const float timeSinceFire, const float launchPower, const float dmgPower) {
	float			fuse{};
	Vector2			velocity;
	float			speed;
	//int				contents;
	int				clipMask{};

	spawnArgs.GetVector("velocity", "0 0 0", velocity);

	speed = velocity.Length() * launchPower;

	//damagePower = dmgPower;

	fuse = spawnArgs.GetFloat("fuse");

	//contents = 0;
	clipMask = MASK_SOLID;

	Vector2 tmpDir = dir;
	tmpDir.Normalize();

	//physicsObj->SetContents(contents);
	physicsObj->SetClipMask(clipMask);
	physicsObj->SetLinearVelocity(tmpDir * speed + pushVelocity);
	physicsObj->SetOrigin(start);

	if (fuse <= 0) {
		// run physics for 1 second
		RunPhysics();
		PostEventMS(&EV_Remove, spawnArgs.GetInt("remove_time", "1500"));
	}
	else if (spawnArgs.GetBool("detonate_on_fuse")) {
		fuse -= timeSinceFire;
		if (fuse < 0.0f) {
			fuse = 0.0f;
		}
		PostEventSec(&EV_Explode, fuse);
	}

	UpdateVisuals();

	state = projectileState_t::LAUNCHED;
}

/*
================
idProjectile::Think
================
*/
void idProjectile::Think() {
	// run physics
	RunPhysics();

	Present();
}

/*
=================
idProjectile::Collide
=================
*/
bool idProjectile::Collide(const trace_t& collision, const Vector2& velocity) noexcept {
	idEntity* ignore;
	std::string damageDefName;
	Vector2		dir{};
	float		damageScale{};

	if (state == projectileState_t::EXPLODED || state == projectileState_t::FIZZLED) {
		return true;
	}

	const bool isHitscan = spawnArgs.GetBool("net_instanthit");

	// get the entity the projectile collided with
	std::shared_ptr<idEntity> ent = gameLocal.entities[collision.c.entityNum];
	if (ent.get() == owner) {
		assert(0);
		return true;
	}

	// direction of projectile
	dir = velocity;
	dir.Normalize();

	SetOrigin(collision.endpos);

	// unlink the clip model because we no longer need it
	GetPhysics()->UnlinkClip();

	damageDefName = spawnArgs.GetString("def_damage");

	ignore = nullptr;

	// if the hit entity takes damage
	if (ent->fl.takedamage) {
		/*if (damagePower) {
			damageScale = damagePower;
		}
		else {
			damageScale = 1.0f;
		}*/

		if (!damageDefName.empty()) {
			ent->Damage(this, owner, dir, damageDefName, damageScale);

			ignore = ent.get();
		}
	}

	Explode(collision, ignore);

	return true;
}

/*
================
idProjectile::Explode
================
*/
void idProjectile::Explode(const trace_t& collision, idEntity* ignore) {
	int removeTime;

	if (state == projectileState_t::EXPLODED || state == projectileState_t::FIZZLED) {
		return;
	}

	Hide();

	// default remove time
	removeTime = spawnArgs.GetInt("remove_time", "1500");

	fl.takedamage = false;
	//physicsObj->SetContents(0);
	physicsObj->PutToRest();

	state = projectileState_t::EXPLODED;

	CancelEvents(&EV_Explode);
	PostEventMS(&EV_Remove, removeTime);
}

/*
================
idProjectile::Event_Explode
================
*/
void idProjectile::Event_Explode() {
	trace_t collision;

	memset(&collision, 0, sizeof(collision));
	collision.endpos = GetPhysics()->GetOrigin();
	collision.c.point = GetPhysics()->GetOrigin();
	Explode(collision, NULL);
}