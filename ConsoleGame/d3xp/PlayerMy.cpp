#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "../framework/Common_local.h"

CLASS_DECLARATION(idPlayer, PlayerMy)
END_CLASS

PlayerMy::PlayerMy() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "PlayerMy");
#endif // DEBUG_PRINT_Ctor_Dtor
}

PlayerMy::~PlayerMy() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "PlayerMy");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
==============
PlayerMy::Init
==============
*/
void PlayerMy::Init() {
}

/*
==============
PlayerMy::Spawn

Prepare any resources used by the player.
==============
*/
void PlayerMy::Spawn() {
	// set our collision model
	physicsObj = std::make_shared<Physics_PlayerMy>();
	physicsObj->SetSelf(shared_from_this());
	SetClipModel();
	//physicsObj.SetMass(spawnArgs.GetFloat("mass", "100"));
	//physicsObj->SetContents(CONTENTS_BODY);
	physicsObj->SetClipMask(MASK_SOLID);
	SetPhysics(physicsObj);

	SpawnFromSpawnSpot();
}

/*
===========
PlayerMy::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void PlayerMy::SelectInitialSpawnPoint(Vector2& origin, Vector2& angles) {
	origin = gameLocal.SelectInitialSpawnPoint(std::dynamic_pointer_cast<PlayerMy>(shared_from_this()));
	angles = vec2_origin;
}

/*
===========
PlayerMy::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void PlayerMy::SpawnFromSpawnSpot() {
	Vector2 spawn_origin;
	Vector2 spawn_angles;

	SelectInitialSpawnPoint(spawn_origin, spawn_angles);
	SpawnToPoint(spawn_origin, spawn_angles);
}

/*
===========
PlayerMy::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void PlayerMy::SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles) {
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

	BecomeActive(TH_THINK);

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	Think();
}

/*
==============
PlayerMy::Collide
==============
*/
bool PlayerMy::Collide(const trace_t& collision, const Vector2& velocity) {
	auto other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (other->IsType(idSimpleObject::Type) || other->IsType(idChain::Type)) {
			other->PostEventMS(&EV_Remove, 0);
			gameLocal.AddRandomPoint();

			return true;
		}
		else if (other->IsType(idChain::Type)) {
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
idPlayer::SetClipModel
==============
*/
void PlayerMy::SetClipModel() {
	physicsObj->SetClipModel(GetPhysics()->GetClipModel(), 1.0f);
}

/*
==============
PlayerMy::Think

Called every tic for each player
==============
*/
void PlayerMy::Think() {
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
PlayerMy::EvaluateControls
==============
*/
void PlayerMy::EvaluateControls() {
	/*if (usercmd.impulseSequence != oldImpulseSequence) {
		PerformImpulse(usercmd.impulse);
	}

	oldImpulseSequence = usercmd.impulseSequence;*/

	AdjustSpeed();
}

/*
==============
PlayerMy::AdjustSpeed
==============
*/
void PlayerMy::AdjustSpeed() {
	float speed{};

	if (usercmd.buttons & BUTTON_RUN) {
		speed = pm_runspeed.GetFloat();
	}
	else {
		speed = pm_walkspeed.GetFloat();
	}

	physicsObj->SetSpeed(speed, 0.0f);
}

/*
==============
PlayerMy::Move
==============
*/
void PlayerMy::Move() {
	Vector2 oldOrigin;
	Vector2 oldVelocity;
	//Vector2 pushVelocity;

	// save old origin and velocity for crashlanding
	oldOrigin = physicsObj->GetOrigin();
	oldVelocity = physicsObj->GetLinearVelocity();
	//pushVelocity = physicsObj->GetPushedLinearVelocity();

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