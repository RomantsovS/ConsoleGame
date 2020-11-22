#include "PlayerChain.h"
#include "Game_local.h"
#include "gamesys/SysCvar.h"
#include "Misc.h"
#include "AFEntity.h"

CLASS_DECLARATION(idPlayer, PlayerChain)
END_CLASS

PlayerChain::PlayerChain() {
}

PlayerChain::~PlayerChain() {
}

/*
==============
PlayerChain::Init
==============
*/
void PlayerChain::Init() {
}

/*
==============
PlayerChain::Spawn

Prepare any resources used by the player.
==============
*/
void PlayerChain::Spawn() {
	// set our collision model
	physicsObj = std::make_shared<idPhysics_Player>();
	physicsObj->SetSelf(shared_from_this());
	physicsObj->SetClipMask(MASK_SOLID);
	SetPhysics(physicsObj);
	SetClipModel();

	SpawnFromSpawnSpot();
}

/*
===========
PlayerChain::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void PlayerChain::SelectInitialSpawnPoint(Vector2& origin, Vector2& angles) {
	origin = gameLocal.SelectInitialSpawnPoint(std::dynamic_pointer_cast<PlayerChain>(shared_from_this()));
	angles = vec2_origin;
}

/*
===========
PlayerChain::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void PlayerChain::SpawnFromSpawnSpot() {
	Vector2 spawn_origin;
	Vector2 spawn_angles;

	SelectInitialSpawnPoint(spawn_origin, spawn_angles);
	SpawnToPoint(spawn_origin, spawn_angles);
}

/*
===========
PlayerChain::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void PlayerChain::SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles) {
	Init();

	// set back the player physics
	SetPhysics(physicsObj);

	physicsObj->SetClipModelAxis();
	physicsObj->EnableClip();

	Vector2 linearVelocity;
	spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
	physicsObj->SetLinearVelocity(linearVelocity);

	// setup our initial view
	SetOrigin(spawn_origin);

	physicsObj->SetClipMask(MASK_SOLID); // the clip mask is usually maintained in Move(), but KillBox requires it

	BecomeActive(TH_THINK);

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	Think();
}

/*
==============
PlayerChain::Collide
==============
*/
bool PlayerChain::Collide(const trace_t& collision, const Vector2& velocity) {
	auto other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (other->IsType(idSimpleObject::Type) || other->IsType(idChain::Type)) {
			other->PostEventMS(&EV_Remove, 0);
			return true;
		}
		else if (other->IsType(idStaticEntity::Type)) {
		}
	}
	return false;
}

/*
==============
PlayerChain::Think

Called every tic for each player
==============
*/
void PlayerChain::Think() {
	/*playedTimeResidual += (gameLocal.time - gameLocal.previousTime);
	playedTimeSecs += playedTimeResidual / 1000;
	playedTimeResidual = playedTimeResidual % 1000;

	buttonMask &= usercmd.buttons;
	usercmd.buttons &= ~buttonMask;*/

	EvaluateControls();

	Move();

	Present();

	if (!(thinkFlags & TH_THINK)) {
		gameLocal.Printf("player %d not thinking?\n", entityNumber);
	}
}

/*
==============
PlayerChain::EvaluateControls
==============
*/
void PlayerChain::EvaluateControls() {
	/*if (usercmd.impulseSequence != oldImpulseSequence) {
		PerformImpulse(usercmd.impulse);
	}

	oldImpulseSequence = usercmd.impulseSequence;*/

	AdjustSpeed();
}

/*
==============
PlayerChain::AdjustSpeed
==============
*/
void PlayerChain::AdjustSpeed() {
	auto speed = pm_walkspeed.GetFloat();
	physicsObj->SetSpeed(speed, 0.0f);
}

/*
==============
PlayerChain::Move
==============
*/
void PlayerChain::Move() {
	Vector2 oldOrigin;
	Vector2 oldVelocity;
	//Vector2 pushVelocity;

	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj->GetOrigin();
	oldVelocity = physicsObj->GetLinearVelocity();
	//pushVelocity = physicsObj->GetPushedLinearVelocity();

	//physicsObj.SetContents(CONTENTS_BODY);
	//physicsObj.SetMovementType(PM_NORMAL);

	physicsObj->SetClipMask(MASK_SOLID);

	{
		//Vector2	org;
		//idMat3	axis;
		//GetViewPos(org, axis);

		physicsObj->SetPlayerInput(usercmd, vec2_origin);
	}

	// FIXME: physics gets disabled somehow
	BecomeActive(TH_PHYSICS);
	RunPhysics();
}