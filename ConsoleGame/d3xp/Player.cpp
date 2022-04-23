#include <precompiled.h>
#pragma hdrstop

#include "Game_local.h"
#include "../framework/Common_local.h"

constexpr int MAX_WEAPONS = 32;

CLASS_DECLARATION(idActor, idPlayer)
END_CLASS

idPlayer::idPlayer() {
}

idPlayer::~idPlayer() {
}

/*
==============
idPlayer::SetupWeaponEntity
==============
*/
void idPlayer::SetupWeaponEntity() {
	int w{};
	std::string weap;

	if (weapon) {
		// get rid of old weapon
		weapon->Clear();
		currentWeapon = -1;
	}
	else {
		weapon = std::static_pointer_cast<idWeapon>(gameLocal.SpawnEntityType(idWeapon::Type, nullptr));
		weapon->SetOwner(dynamic_cast<idPlayer*>(this));
		currentWeapon = -1;
	}

	for (w = 0; w < MAX_WEAPONS; w++) {
		weap = spawnArgs.GetString(va("def_weapon%d", w));
		if (!weap.empty()) {
			idWeapon::CacheWeapon(weap);
		}
	}
}

/*
==============
idPlayer::Init
==============
*/
void idPlayer::Init() {
	oldButtons = 0;
	
	currentWeapon = -1;
	idealWeapon = -1;

	// damage values
	fl.takedamage = true;

	// restore persistent data
	RestorePersistantInfo();

	SetupWeaponEntity();
	currentWeapon = -1;
}

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void idPlayer::Spawn() {
	// set our collision model
	physicsObj = std::make_shared<Physics_PlayerMy>();
	physicsObj->SetSelf(this);
	SetClipModel();
	//physicsObj.SetMass(spawnArgs.GetFloat("mass", "100"));
	//physicsObj->SetContents(CONTENTS_BODY);
	physicsObj->SetClipMask(MASK_SOLID);
	SetPhysics(physicsObj);

	SetupWeaponEntity();
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
	origin = gameLocal.SelectInitialSpawnPoint(dynamic_cast<idPlayer*>(this));
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

	BecomeActive(TH_THINK);

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	Think();
}

/*
===============
idPlayer::RestorePersistantInfo

Restores any inventory and player stats when changing levels.
===============
*/
void idPlayer::RestorePersistantInfo() {
	idealWeapon = spawnArgs.GetInt("current_weapon", "0");
}

/*
===============
idPlayer::FireWeapon
===============
*/
idCVar g_infiniteAmmo("g_infiniteAmmo", "1", CVAR_GAME | CVAR_BOOL, "infinite ammo");

void idPlayer::FireWeapon() noexcept {
	if (weapon->IsReady()) {
		if (g_infiniteAmmo.GetBool()) {
			//AI_ATTACK_HELD = true;
			weapon->BeginAttack();
		}
		else {
		}
	}
}

/*
===============
idPlayer::Weapon_Combat
===============
*/
void idPlayer::Weapon_Combat() {
	if (idealWeapon != currentWeapon && idealWeapon < MAX_WEAPONS) {
		/*if (weapon.GetEntity()->IsReady()) {
			weapon.GetEntity()->PutAway();
		}*/

		if (weapon->IsHolstered()) {
			assert(idealWeapon >= 0);
			assert(idealWeapon < MAX_WEAPONS);

			currentWeapon = idealWeapon;
			std::string animPrefix = spawnArgs.GetString(va("def_weapon%d", currentWeapon));
			weapon->GetWeaponDef(animPrefix, 0);
			weapon->SetStatus(weaponStatus_t::WP_READY);
		}
	}

	// check for attack
	//AI_WEAPON_FIRED = false;

	if ((usercmd.buttons & BUTTON_ATTACK)) {
		FireWeapon();
	}
	else if (oldButtons & BUTTON_ATTACK) {
		//AI_ATTACK_HELD = false;
		weapon->EndAttack();
	}
}

/*
===============
idPlayer::UpdateWeapon
===============
*/
void idPlayer::UpdateWeapon() {
	Weapon_Combat();

	// update weapon state, particles, dlights, etc
	weapon->PresentWeapon();
}

/*
==============
idPlayer::Collide
==============
*/
bool idPlayer::Collide(const trace_t& collision, const Vector2& velocity) noexcept {
	auto other = gameLocal.entities[collision.c.entityNum];
	if (other) {
		if (other->IsType(AISimple::Type) || other->IsType(idChain::Type)) {
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
void idPlayer::SetClipModel() {
	physicsObj->SetClipModel(GetPhysics()->GetClipModel(), 1.0f);
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

	UpdateWeapon();

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
void idPlayer::EvaluateControls() noexcept {
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
void idPlayer::AdjustSpeed() noexcept {
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
void idPlayer::HandleUserCmds(const usercmd_t& newcmd) noexcept {
	// latch button actions
	oldButtons = usercmd.buttons;

	// grab out usercmd
	//oldCmd = usercmd;
	//oldImpulseSequence = usercmd.impulseSequence;
	usercmd = newcmd;
}

bool idPlayer::IsLocallyControlled() const noexcept {
	return entityNumber == gameLocal.GetLocalClientNum();
}