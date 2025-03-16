#include "idlib/precompiled.h"

#include "Game_local.h"
#include "../framework/Common_local.h"

constexpr int MAX_WEAPONS = 32;

CLASS_DECLARATION(idActor, idPlayer)
END_CLASS

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
  } else {
    weapon = std::static_pointer_cast<idWeapon>(
        gameLocal.SpawnEntityType(idWeapon::Type, nullptr));
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
===========
idPlayer::SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
void idPlayer::SelectInitialSpawnPoint(Vector2& origin, Vector2& angles) {
  auto* spot = gameLocal.SelectInitialSpawnPoint(dynamic_cast<idPlayer*>(this));

  origin = spot->GetPhysics()->GetOrigin();
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
void idPlayer::SpawnToPoint(const Vector2& spawn_origin,
                            const Vector2& spawn_angles) {
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

  // Show();

  // set our respawn time and buttons so that if we're killed we don't respawn
  // immediately
  minRespawnTime = gameLocal.time;

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

void idPlayer::UpdateConditions() {
  static std::string cur_anim;
  if (usercmd.rightmove > 0) {
    if (cur_anim != "walk_right") {
      cur_anim = "walk_right";
      auto anim = GetAnim(cur_anim);

      animator.CycleAnim(anim, gameLocal.time);
    }
  } else if (usercmd.rightmove < 0) {
    if (cur_anim != "walk_left") {
      cur_anim = "walk_left";
      auto anim = GetAnim(cur_anim);

      animator.CycleAnim(anim, gameLocal.time);
    }
  } else if (usercmd.rightmove == 0 && usercmd.forwardmove == 0) {
    if (cur_anim.substr(0, 4) != "idle") {
      cur_anim = cur_anim == "walk_left" ? "idle_left" : "idle";
      auto anim = GetAnim(cur_anim);

      animator.CycleAnim(anim, gameLocal.time);
    }
  } else if (usercmd.forwardmove != 0) {
    if (cur_anim.substr(0, 4) != "walk") {
      cur_anim = cur_anim == "idle" ? "walk_right" : "walk_left";
      auto anim = GetAnim(cur_anim);

      animator.CycleAnim(anim, gameLocal.time);
    }
  }
}

/*
===============
idPlayer::FireWeapon
===============
*/
idCVar g_infiniteAmmo("g_infiniteAmmo", "1", CVAR_GAME | CVAR_BOOL,
                      "infinite ammo");

void idPlayer::FireWeapon() noexcept {
  if (weapon->IsReady()) {
    if (g_infiniteAmmo.GetBool()) {
      // AI_ATTACK_HELD = true;
      weapon->BeginAttack();
    } else {
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
      idassert(idealWeapon >= 0);
      idassert(idealWeapon < MAX_WEAPONS);

      currentWeapon = idealWeapon;
      std::string animPrefix =
          spawnArgs.GetString(va("def_weapon%d", currentWeapon));
      weapon->GetWeaponDef(animPrefix, 0);
      weapon->SetStatus(weaponStatus_t::WP_READY);
    }
  }

  // check for attack
  // AI_WEAPON_FIRED = false;

  if ((usercmd.buttons & BUTTON_ATTACK)) {
    FireWeapon();
  } else if (oldButtons & BUTTON_ATTACK) {
    // AI_ATTACK_HELD = false;
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
bool idPlayer::Collide(const trace_t& collision,
                       const Vector2& velocity) noexcept {
  auto& other = gameLocal.entities[collision.c.entityNum];
  if (other) {
    if (other->IsType(AISimple::Type)) {
      const std::string& damageDefName =
          other->spawnArgs.GetString("def_damage");

      Damage(other.get(), other.get(), vec2_origin, damageDefName);

      return true;
    } else if (other->IsType(idChain::Type)) {
      other->PostEventMS(&EV_Remove, 0);
      return true;
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

  UpdateConditions();

  // this may use firstPersonView, or a thirdPeroson / camera view
  CalculateRenderView();

  UpdateWeapon();

  UpdateAnimation();

  Present();

  if (!(thinkFlags & TH_THINK)) {
    gameLocal.Printf("player %d not thinking?\n", entityNumber);
  }
}

/*
==================
idPlayer::Killed
==================
*/
void idPlayer::Killed(idEntity* inflictor, idEntity* attacker, int damage,
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

/*
=================
idPlayer::CalcDamagePoints

Calculates how many health and armor points will be inflicted, but
doesn't actually do anything with them.  This is used to tell when an attack
would have killed the player, possibly allowing a "saving throw"
=================
*/
void idPlayer::CalcDamagePoints(idEntity* inflictor, idEntity* attacker,
                                const idDict* damageDef, int* health) {
  int damage;

  damageDef->GetInt("damage", "20", damage);

  *health = damage;
}

/*
==============
idPlayer::EvaluateControls
==============
*/
void idPlayer::EvaluateControls() noexcept {
  // check for respawning
  if (health <= 0 && !g_testDeath.GetBool()) {
    /*if (common->IsMultiplayer()) {
            // in MP, idMultiplayerGame decides spawns
            if ((gameLocal.time > minRespawnTime) && (usercmd.buttons &
    BUTTON_ATTACK)) { forceRespawn = true;
            }
            else if (gameLocal.time > maxRespawnTime) {
                    forceRespawn = true;
            }
    }
    else {*/
    // in single player, we let the session handle restarting the level or
    // loading a game
    if (gameLocal.time > minRespawnTime) {
      gameLocal.sessionCommand = "died";
    }
    //}
  }

  /*if (usercmd.impulseSequence != oldImpulseSequence) {
          PerformImpulse(usercmd.impulse);
  }

  oldImpulseSequence = usercmd.impulseSequence;*/

  AdjustSpeed();
}

/*
============
ServerDealDamage

Only called on the server and in singleplayer. This is where
the player's health is actually modified, but the visual and
sound effects happen elsewhere so that clients can get instant
feedback and hide lag.
============
*/
void idPlayer::ServerDealDamage(int damage, idEntity& inflictor,
                                idEntity& attacker, const Vector2& dir,
                                const std::string& damageDefName) {
  const auto damageDef = gameLocal.FindEntityDef(damageDefName, false);
  if (!damageDef) {
    gameLocal.Warning("Unknown damageDef '%s'", damageDefName.c_str());
    return;
  }

  // do the damage
  if (damage > 0) {
    int oldHealth = health;
    health -= damage;

    if (health <= 0) {
      if (health < -999) {
        health = -999;
      }
      Killed(&inflictor, &attacker, damage, dir);
    }
  }
}

/*
============
Damage

this		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
        example: this=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback in global space

damageDef	an idDict with all the options for damage effects

inflictor, attacker, dir, and point can be NULL for environmental effects
============
*/
void idPlayer::Damage(idEntity* inflictor, idEntity* attacker,
                      const Vector2& dir, const std::string& damageDefName) {
  int damage;

  if (health <= 0) return;

  if (!fl.takedamage) {
    return;
  }

  if (!inflictor) {
    inflictor = gameLocal.world.get();
  }
  if (!attacker) {
    attacker = gameLocal.world.get();
  }

  auto damageDef = gameLocal.FindEntityDef(damageDefName, false);
  if (!damageDef) {
    gameLocal.Warning("Unknown damageDef '%s'", damageDefName.c_str());
    return;
  }

  if (damageDef->dict.GetBool("ignore_player")) {
    return;
  }

  CalcDamagePoints(inflictor, attacker, &damageDef->dict, &damage);

  if (g_debugDamage.GetInteger()) {
    gameLocal.Printf(
        "inflictor:%02d\ttarget:%02d\tdamage "
        "type:%s\t\thealth:%03d\tdamage:%03d\n",
        inflictor->entityNumber, entityNumber, damageDefName.c_str(), health,
        damage);
  }

  // Server will deal his damage normally
  ServerDealDamage(damage, *inflictor, *attacker, dir, damageDefName);
}

std::shared_ptr<renderView_t> idPlayer::GetRenderView() { return renderView; }

void idPlayer::CalculateRenderView() {
  if (!renderView) {
    renderView = std::make_shared<renderView_t>();
  }

  renderView->time[0] = gameLocal.slow.time;
  renderView->time[1] = gameLocal.fast.time;

  renderView->viewID = 0;

  if (g_stopTime.GetBool()) {
  } else {
    // set the viewID to the clientNum + 1, so we can suppress the right
    // player bodies and allow the right player view weapons
    renderView->viewID = entityNumber + 1;
  }
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
  } else {
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
  // Vector2 pushVelocity;

  // save old origin and velocity for crashlanding
  oldOrigin = physicsObj->GetOrigin();
  oldVelocity = physicsObj->GetLinearVelocity();
  // pushVelocity = physicsObj->GetPushedLinearVelocity();

  if (health <= 0) {
    physicsObj->SetContents(
        static_cast<int>(contentsFlags_t::CONTENTS_CORPSE) |
        static_cast<int>(contentsFlags_t::CONTENTS_MONSTERCLIP));
  } else {
    physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_BODY));
  }

  if (health <= 0) {
    physicsObj->SetClipMask(MASK_DEADSOLID);
  } else {
    physicsObj->SetClipMask(MASK_PLAYERSOLID);
  }

  {
    // Vector2	org;
    // idMat3	axis;
    // GetViewPos(org, axis);

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
  // oldCmd = usercmd;
  // oldImpulseSequence = usercmd.impulseSequence;
  usercmd = newcmd;
}

/*
================
idPlayer::ClientThink
================
*/
void idPlayer::ClientThink(
    /*const int curTime, const float fraction, const bool predict*/) {
  AdjustSpeed();

  // if (!isLagged) {
  //	// don't allow client to move when lagged
  //	if (IsLocallyControlled()) {
  //		// Locally-controlled clients are authoritative on their
  // positions, so they can move normally. 		Move();
  // usercmd.pos = physicsObj.GetOrigin();
  //	}
  //	else {
  //		// Non-locally controlled players are interpolated.
  //		Move_Interpolated(fraction);
  //	}
  // }

  // this may use firstPersonView, or a thirdPerson / camera view
  CalculateRenderView();

  UpdateAnimation();

  Present();
}

/*
================
idPlayer::WriteToSnapshot
================
*/
void idPlayer::WriteToSnapshot(idBitMsg& msg) const {
  physicsObj->WriteToSnapshot(msg);
}

/*
================
idPlayer::ReadFromSnapshot
================
*/
void idPlayer::ReadFromSnapshot(const idBitMsg& msg) {
  physicsObj->ReadFromSnapshot(msg);

  if (true /*msg.HasChanged()*/) {
    UpdateVisuals();
  }
}

bool idPlayer::IsLocallyControlled() const noexcept {
  return entityNumber == gameLocal.GetLocalClientNum();
}
