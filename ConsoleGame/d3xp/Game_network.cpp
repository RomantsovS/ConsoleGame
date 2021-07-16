#include <precompiled.h>
#pragma hdrstop

#include "Game_local.h"
#include "..\framework\Common_local.h"

/*
================
idGameLocal::SyncPlayersWithLobbyUsers
================
*/
void idGameLocal::SyncPlayersWithLobbyUsers(bool initial) {
	// spawn the player
	SpawnPlayer(0);
}