
#include "idlib/precompiled.h"

#include "../Game_local.h"

idCVar si_map("si_map", "-1", CVAR_GAME | CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_INTEGER, "default map choice for profile");

// change anytime vars
idCVar developer("developer", "0", CVAR_GAME | CVAR_BOOL, "");

idCVar g_debugDamage("g_debugDamage", "0", CVAR_GAME | CVAR_BOOL, "");
idCVar g_debugSpawn("g_debugSpawn", "0", CVAR_GAME | CVAR_BOOL, "");

idCVar g_showCollisionModels("g_showCollisionModels", "0", CVAR_GAME | CVAR_BOOL, "");
idCVar g_showCollisionTraces("g_showCollisionTraces", "0", CVAR_GAME | CVAR_BOOL, "");
idCVar g_maxShowDistance("g_maxShowDistance", "128", CVAR_GAME | CVAR_FLOAT, "");
idCVar g_showEntityInfo("g_showEntityInfo", "0", CVAR_GAME | CVAR_BOOL, "");

idCVar pm_walkspeed("pm_walkspeed", "10", CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while walking");
idCVar pm_runspeed("pm_runspeed", "20", CVAR_GAME | CVAR_NETWORKSYNC | CVAR_FLOAT, "speed the player can move while running");

idCVar g_testDeath("g_testDeath", "0", CVAR_GAME | CVAR_BOOL, "");