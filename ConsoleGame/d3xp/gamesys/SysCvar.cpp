#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "../Game_local.h"

idCVar g_showCollisionModels("g_showCollisionModels", "0", CVAR_GAME | CVAR_BOOL, "");
idCVar g_maxShowDistance("g_maxShowDistance", "128", CVAR_GAME | CVAR_FLOAT, "");
idCVar g_showCollisionTraces("g_showCollisionTraces", "0", CVAR_GAME | CVAR_BOOL, "");

idCVar pm_walkspeed("pm_walkspeed", "1", CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while walking");