#include "Player.h"
#include "Game_local.h"
#include "gamesys/SysCvar.h"
#include "Misc.h"

CLASS_DECLARATION(idActor, idPlayer)
END_CLASS

idPlayer::idPlayer() {
}

idPlayer::~idPlayer() {
}

/*
==============
idPlayer::Init
==============
*/
void idPlayer::Init() {
}

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void idPlayer::Spawn() {
	// set our collision model
	physicsObj = std::make_shared<idPhysics_Player>();
	physicsObj->SetSelf(shared_from_this());
	SetClipModel();
	physicsObj->SetClipMask(MASK_SOLID);
	SetPhysics(physicsObj);

	SpawnFromSpawnSpot();
}

/*
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint(Vector2& origin, Vector2& angles) {
	origin = gameLocal.SelectInitialSpawnPoint(std::dynamic_pointer_cast<idPlayer>(shared_from_this()));
	angles = vec2_origin;
}

/*
===========
idPlayer::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void idPlayer::SpawnFromSpawnSpot() {
	Vector2 spawn_origin;
	Vector2 spawn_angles;

	SelectInitialSpawnPoint(spawn_origin, spawn_angles);
	SpawnToPoint(spawn_origin, spawn_angles);
}

/*
===========
idPlayer::SpawnToPoint

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState

when called here with spectating set to true, just place yourself and init
============
*/
void idPlayer::SpawnToPoint(const Vector2& spawn_origin, const Vector2& spawn_angles) {
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
idPlayer::Collide
==============
*/
bool idPlayer::Collide(const trace_t& collision, const Vector2& velocity) {
	auto other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (other->IsType(idSimpleObject::Type)) {
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
void idPlayer::SetClipModel() {
	/*idBounds bounds;
	bounds.Zero();
	bounds.AddPoint(Vector2(1, 1));

	auto newClip = std::make_shared<idClipModel>(idTraceModel(bounds));
	newClip->Translate(physicsObj->PlayerGetOrigin());*/

	idTraceModel trm;
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

	auto newClip = std::make_shared<idClipModel>(trm);

	physicsObj->SetClipModel(newClip, 0.0f);
}

/*
==============
idPlayer::Think

Called every tic for each player
==============
*/
void idPlayer::Think() {
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
idPlayer::EvaluateControls
==============
*/
void idPlayer::EvaluateControls() {
	/*if (usercmd.impulseSequence != oldImpulseSequence) {
		PerformImpulse(usercmd.impulse);
	}

	oldImpulseSequence = usercmd.impulseSequence;*/

	AdjustSpeed();
}

/*
==============
idPlayer::AdjustSpeed
==============
*/
void idPlayer::AdjustSpeed() {
	auto speed = pm_walkspeed.GetFloat();
	physicsObj->SetSpeed(speed, 0.0f);
}

/*
==============
idPlayer::Move
==============
*/
void idPlayer::Move() {
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

/*
================
idPlayer::HandleUserCmds
================
*/
void idPlayer::HandleUserCmds(const usercmd_t& newcmd) {
	// latch button actions
	//oldButtons = usercmd.buttons;

	// grab out usercmd
	//oldCmd = usercmd;
	//oldImpulseSequence = usercmd.impulseSequence;
	usercmd = newcmd;
}

bool idPlayer::IsLocallyControlled() const {
	return entityNumber == gameLocal.GetLocalClientNum();
}