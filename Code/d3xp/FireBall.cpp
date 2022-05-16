#include "idlib/precompiled.h"

#include "Game_local.h"

CLASS_DECLARATION(idProjectile, FireBall)
END_CLASS

/*
================
FireBall::Spawn
================
*/
void FireBall::Spawn() {
	physicsObj = std::make_shared<idPhysics_RigidBody>();
	physicsObj->SetSelf(this);
	physicsObj->SetClipModel(std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), 1.0f);
	physicsObj->SetContents(0);
	physicsObj->SetClipMask(0);
	SetPhysics(physicsObj);
}

/*
=================
FireBall::Launch
=================
*/
void FireBall::Launch(const Vector2& start, const Vector2& dir, const Vector2& pushVelocity, const float timeSinceFire, const float launchPower, const float dmgPower) {
	float			fuse{};
	Vector2			velocity;
	float			speed;

	spawnArgs.GetVector("velocity", "0 0 0", velocity);

	speed = velocity.Length() * launchPower;


	fuse = spawnArgs.GetFloat("fuse");

	Vector2 tmpDir = dir;
	tmpDir.Normalize();

	physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_PROJECTILE));
	physicsObj->SetClipMask(MASK_PLAYERSOLID);
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
FireBall::Think
================
*/
void FireBall::Think() {
	// run physics
	RunPhysics();

	Present();
}

/*
================
FireBall::Explode
================
*/
void FireBall::Explode(const trace_t& collision, idEntity* ignore) {
	int removeTime;

	if (state == projectileState_t::EXPLODED || state == projectileState_t::FIZZLED) {
		return;
	}

	Hide();

	// default remove time
	removeTime = spawnArgs.GetInt("remove_time", "1500");

	fl.takedamage = false;
	physicsObj->SetContents(0);
	physicsObj->PutToRest();

	state = projectileState_t::EXPLODED;

	CancelEvents(&EV_Explode);
	PostEventMS(&EV_Remove, removeTime);
}