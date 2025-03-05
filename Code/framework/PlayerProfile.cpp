#include "idlib/precompiled.h"
#include "PlayerProfile.h"
#include "CmdSystem.h"

class idPlayerProfileLocal : public idPlayerProfile {};
idPlayerProfileLocal playerProfiles[1];

/*
========================
Contains data that needs to be saved out on a per player profile basis, global
for the lifetime of the player so the data can be shared across computers.
- HUD tint colors
- key bindings
- etc...
========================
*/

/*
========================
idPlayerProfile * CreatePlayerProfile
========================
*/
idPlayerProfile* idPlayerProfile::CreatePlayerProfile(int deviceIndex) {
  playerProfiles[deviceIndex].SetDefaults();
  playerProfiles[deviceIndex].deviceNum = deviceIndex;
  return &playerProfiles[deviceIndex];
}

/*
========================
idPlayerProfile::idPlayerProfile
========================
*/
idPlayerProfile::idPlayerProfile() {
  SetDefaults();

  // Don't have these in SetDefaults because they're used for state management
  // and SetDefaults is called when loading the profile
  state = IDLE;
  requestedState = IDLE;
  deviceNum = -1;
  // dirty = false;
}

/*
========================
idPlayerProfile::SetDefaults
========================
*/
void idPlayerProfile::SetDefaults() {
  /*achievementBits = 0;
  achievementBits2 = 0;
  dlcReleaseVersion = 0;

  stats.SetNum(MAX_PLAYER_PROFILE_STATS);
  for (int i = 0; i < MAX_PLAYER_PROFILE_STATS; ++i) {
          stats[i].i = 0;
  }

  leftyFlip = false;*/
  customConfig = false;
  // configSet = 0;
}

/*
========================
idPlayerProfile::~idPlayerProfile
========================
*/
idPlayerProfile::~idPlayerProfile() {}

/*
========================
idPlayerProfile::ExecConfig
========================
*/
void idPlayerProfile::ExecConfig(bool save, bool forceDefault) {
  int flags = 0;
  /*if (!save) {
          flags = cvarSystem->GetModifiedFlags();
  }*/

  if (!customConfig || forceDefault) {
    cmdSystem->AppendCommandText("exec default.cfg\n");
  }

  cmdSystem->ExecuteCommandBuffer();

  /*if (!save) {
          cvarSystem->ClearModifiedFlags(CVAR_ARCHIVE);
          cvarSystem->SetModifiedFlags(flags);
  }*/
}