#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "../framework/Common_local.h"

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
}

/*
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint(Vector2& origin, Vector2& angles) {
}

/*
===========
idPlayer::SpawnFromSpawnSpot

Chooses a spawn location and spawns the player
============
*/
void idPlayer::SpawnFromSpawnSpot() {
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
}

/*
==============
idPlayer::SetClipModel
==============
*/
void idPlayer::SetClipModel() {
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