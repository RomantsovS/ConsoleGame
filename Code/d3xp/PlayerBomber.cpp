#include "idlib/precompiled.h"

#include "Game_local.h"
#include "../framework/Common_local.h"

CLASS_DECLARATION(idPlayer, PlayerBomber)
END_CLASS

/*
==============
idPlayer::Spawn

Prepare any resources used by the player.
==============
*/
void PlayerBomber::Spawn() {
  // set our collision model
  physicsObj = std::make_shared<Physics_PlayerMy>();
  physicsObj->SetSelf(this);
  SetClipModel();
  // physicsObj.SetMass(spawnArgs.GetFloat("mass", "100"));
  physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_BODY));
  physicsObj->SetClipMask(MASK_PLAYERSOLID);
  SetPhysics(physicsObj);

  // init the damage effects
  playerView.SetPlayerEntity(this);

  if (common->IsMultiplayer()) {
    Init();
    if (!common->IsClient()) {
      // set yourself ready to spawn. idMultiplayerGame will decide when/if
      // appropriate and call SpawnFromSpawnSpot
      SetupWeaponEntity();
      SpawnFromSpawnSpot();
    }
  } else {
    SetupWeaponEntity();
    SpawnFromSpawnSpot();
  }
}

/*
==================
idPlayer::Killed
==================
*/
void PlayerBomber::Killed(idEntity* inflictor, idEntity* attacker, int damage,
                      const Vector2& dir) noexcept {
  float delay;

  // don't allow respawn until the death anim is done
  // g_forcerespawn may force spawning at some later time
  delay = spawnArgs.GetFloat("respawn_delay");
  minRespawnTime = gameLocal.time + SEC2MS(delay);

  physicsObj->SetContents(
      static_cast<int>(contentsFlags_t::CONTENTS_CORPSE) |
      static_cast<int>(contentsFlags_t::CONTENTS_MONSTERCLIP));
  physicsObj->SetMovementType(pmtype_t::PM_DEAD);

  Hide();

  UpdateVisuals();
}
