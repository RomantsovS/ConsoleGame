#include "idlib/precompiled.h"

#include "Game_local.h"

const idEventDef EV_Explode("<explode>", "");

CLASS_DECLARATION(idEntity, idProjectile)
EVENT(EV_Explode, idProjectile::Event_Explode)
END_CLASS

/*
================
idProjectile::Spawn
================
*/
void idProjectile::Spawn() {
  physicsObj = std::make_shared<idPhysics_RigidBody>();
  physicsObj->SetSelf(this);
  physicsObj->SetClipModel(
      std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), 1.0f);
  physicsObj->SetContents(0);
  physicsObj->SetClipMask(0);
  SetPhysics(physicsObj);

  // get the projectile
  projectileDict.Clear();

  std::string projectileName = spawnArgs.GetString("def_projectile");
  if (!projectileName.empty()) {
    const std::shared_ptr<idDeclEntityDef>& projectileDef =
        gameLocal.FindEntityDef(projectileName, false);
    if (!projectileDef) {
      gameLocal.Warning("Unknown projectile '%s' in weapon '%s'",
                        projectileName.c_str(), name.c_str());
    } else {
      const std::string& spawnclass =
          projectileDef->dict.GetString("spawnclass");
      idTypeInfo* cls = idClass::GetClass(spawnclass);
      if (!cls || !cls->IsType(idProjectile::Type)) {
        gameLocal.Warning(
            "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')",
            spawnclass.c_str(), projectileName.c_str(), name.c_str());
      } else {
        projectileDict = projectileDef->dict;
      }
    }
  }
}

/*
================
idProjectile::GetOwner
================
*/
idEntity* idProjectile::GetOwner() const { return owner; }

/*
================
idProjectile::Create
================
*/
void idProjectile::Create(idEntity* owner, const Vector2& start,
                          const Vector2& dir) {
  idDict args;

  physicsObj->SetOrigin(start);

  physicsObj->GetClipModel()->SetOwner(owner);

  this->owner = owner;

  UpdateVisuals();

  state = projectileState_t::CREATED;
}

/*
=================
idProjectile::Launch
=================
*/
void idProjectile::Launch(const Vector2& start, const Vector2& dir,
                          const Vector2& pushVelocity,
                          const float timeSinceFire, const float launchPower,
                          const float dmgPower) {
  float fuse{};
  Vector2 velocity;
  float speed;

  fl.takedamage = !spawnArgs.GetBool("noDamage");

  spawnArgs.GetVector("velocity", "0 0 0", velocity);

  speed = velocity.Length() * launchPower;

  fuse = spawnArgs.GetFloat("fuse");

  Vector2 tmpDir = dir;
  tmpDir.Normalize();

  if (spawnArgs.GetBool("solid")) {
    physicsObj->SetContents(
        static_cast<int>(contentsFlags_t::CONTENTS_PROJECTILE));
  } else {
    physicsObj->SetContents(0);
  }
  physicsObj->SetClipMask(MASK_PLAYERSOLID);
  physicsObj->SetLinearVelocity(tmpDir * speed + pushVelocity);
  physicsObj->SetOrigin(start);

  if (g_debugSpawn.GetBool())
    gameLocal.DPrintf("Launch proj %d %s %f %f\n", entityNumber,
                      GetName().c_str(), start.x, start.y);

  if (!common->IsClient() || fl.skipReplication) {
    if (fuse <= 0) {
      // run physics for 1 second
      RunPhysics();
      PostEventMS(&EV_Remove, spawnArgs.GetInt("remove_time", "1500"));
    } else if (spawnArgs.GetBool("detonate_on_fuse")) {
      fuse -= timeSinceFire;
      if (fuse < 0.0f) {
        fuse = 0.0f;
      }
      PostEventSec(&EV_Explode, fuse);
    }
  }

  UpdateVisuals();

  state = projectileState_t::LAUNCHED;
}

/*
================
idProjectile::Think
================
*/
void idProjectile::Think() {
  // run physics
  RunPhysics();

  if (spawnArgs.GetBool("solid")) {
    if (playerTouched != PlayerTouchState::UNTAUCHED) {
      idPlayer* player = gameLocal.GetLocalPlayer();

      if (player) {
        bool isPlayerTouchedNow = false;
        for (size_t i = 0; i < physicsObj->GetNumContacts(); ++i) {
          auto& ci = physicsObj->GetContact(i);
          auto& ent = gameLocal.entities[ci.entityNum];
          if (ent.get() == player) {
            isPlayerTouchedNow = true;
            break;
          }
        }
        if (isPlayerTouchedNow && playerTouched == PlayerTouchState::NONE)
          playerTouched = PlayerTouchState::TOUCHED;
        else if (!isPlayerTouchedNow &&
                 playerTouched == PlayerTouchState::TOUCHED) {
          GetPhysics()->SetContents(
              static_cast<int>(contentsFlags_t::CONTENTS_SOLID));
          playerTouched = PlayerTouchState::UNTAUCHED;
        }
      }
    }
  } else {
    for (size_t i = 0; i < physicsObj->GetNumContacts(); ++i) {
      auto& ci = physicsObj->GetContact(i);
      auto& ent = gameLocal.entities[ci.entityNum];
      DoDamage(ent.get(), vec2_origin);
    }
    if (physicsObj->GetNumContacts() > 0) {
    }
  }

  Present();
}

/*
=================
idProjectile::Collide
=================
*/
bool idProjectile::Collide(const trace_t& collision,
                           const Vector2& velocity) noexcept {
  Vector2 dir{};

  if (state == projectileState_t::EXPLODED ||
      state == projectileState_t::FIZZLED) {
    return true;
  }

  const bool isHitscan = spawnArgs.GetBool("net_instanthit");

  // get the entity the projectile collided with
  idEntity* ent = gameLocal.entities[collision.c.entityNum].get();

  // direction of projectile
  dir = velocity;
  dir.Normalize();

  SetOrigin(collision.endpos);

  // unlink the clip model because we no longer need it
  GetPhysics()->UnlinkClip();

  DoDamage(ent, dir);

  Explode(collision, ent);

  return true;
}

void idProjectile::DoDamage(idEntity* ent, const Vector2& dir) {
  const std::string& damageDefName = spawnArgs.GetString("def_damage");

  // if the hit entity takes damage
  if (ent->fl.takedamage) {
    if (!damageDefName.empty()) {
      ent->Damage(this, owner, dir, damageDefName);
    }
  }
}

/*
================
idProjectile::Explode
================
*/
void idProjectile::Explode(const trace_t& collision, idEntity* ignore) {
  int removeTime;

  if (state == projectileState_t::EXPLODED ||
      state == projectileState_t::FIZZLED) {
    return;
  }

  Hide();

  // default remove time
  removeTime = spawnArgs.GetInt("remove_time", "1500");

  fl.takedamage = false;
  physicsObj->SetContents(0);
  physicsObj->ClearContacts();
  physicsObj->PutToRest();

  if (!common->IsClient() || fl.skipReplication) {
    size_t num_projectile_directions =
        spawnArgs.GetInt("num_projectile_directions");
    size_t num_fireballs = spawnArgs.GetInt("num_fireballs");

    if (!spawnArgs.GetString("def_projectile").empty() && num_fireballs > 0) {
      Vector2 size = spawnArgs.GetVector("size");
      int projShift = projectileDict.GetInt("shift");

      std::shared_ptr<idEntity> ent;
      gameLocal.SpawnEntityDef(projectileDict, &ent);

      if (!ent || !ent->IsType(idProjectile::Type)) {
        const std::string projectileName =
            spawnArgs.GetString("def_projectile");
        gameLocal.Error("'%s' is not an idProjectile", projectileName.c_str());
        return;
      }

      std::shared_ptr<idProjectile> proj =
          std::static_pointer_cast<idProjectile>(ent);

      proj->Create(owner, GetPhysics()->GetOrigin(), vec2_origin);
      proj->Launch(GetPhysics()->GetOrigin(), vec2_origin, vec2_origin);

      for (size_t i = 0; i < num_projectile_directions; ++i) {
        for (size_t j = 0; j < spawnArgs.GetInt("num_fireballs", 1); ++j) {
          Vector2 shift = spawnArgs.GetVector(va("projectile%d_shift", i));
          if (shift.x == 0 && shift.y == 0) continue;
          shift.x *= (size.x + projShift) / 2 * (j + 1);
          shift.y *= (size.y + projShift) / 2 * (j + 1);

          const auto proj_origin = GetPhysics()->GetOrigin() + shift;

          std::shared_ptr<idEntity> ent;
          gameLocal.SpawnEntityDef(projectileDict, &ent);

          if (!ent || !ent->IsType(idProjectile::Type)) {
            const std::string projectileName =
                spawnArgs.GetString("def_projectile");
            gameLocal.Error("'%s' is not an idProjectile",
                            projectileName.c_str());
            return;
          }

          std::shared_ptr<idProjectile> proj =
              std::static_pointer_cast<idProjectile>(ent);

          proj->Create(owner, proj_origin, vec2_origin);
          proj->Launch(proj_origin, vec2_origin, vec2_origin);

          proj->Think();

          bool collided = false;
          for (int i = 0; i < proj->physicsObj->GetNumContacts(); ++i) {
            auto ent =
                gameLocal.entities[proj->physicsObj->GetContact(i).entityNum]
                    .get();
            if (gameLocal.entities[proj->physicsObj->GetContact(i).entityNum]
                    ->IsType(idStaticEntity::Type)) {
              gameLocal.DPrintf("contact proj %d %s %f %f with %d %s %f %f\n",
                                proj->entityNumber, proj->GetName().c_str(),
                                proj->GetPhysics()->GetOrigin().x,
                                proj->GetPhysics()->GetOrigin().y,
                                ent->entityNumber, ent->GetName().c_str(),
                                ent->GetPhysics()->GetOrigin().x,
                                ent->GetPhysics()->GetOrigin().y);
              if (!ent->fl.takedamage) proj->PostEventSec(&EV_Explode, 0);
              collided = true;
              break;
            }
          }
          if (collided) break;
        }
      }
    }
  }

  state = projectileState_t::EXPLODED;

  CancelEvents(&EV_Explode);
  PostEventMS(&EV_Remove, removeTime);
}

/*
================
idProjectile::Event_Explode
================
*/
void idProjectile::Event_Explode() {
  trace_t collision;

  memset(&collision, 0, sizeof(collision));
  collision.endpos = GetPhysics()->GetOrigin();
  // collision.c.point = GetPhysics()->GetOrigin();
  Explode(collision, NULL);
}

void idProjectile::CacheProjectille(const std::string& objectname,
                                    idDeclEntityDef* declEntDef) {
  std::string projectileName = declEntDef->dict.GetString("def_projectile");
  if (!projectileName.empty()) {
    const std::shared_ptr<idDeclEntityDef>& projectileDef =
        gameLocal.FindEntityDef(projectileName, false);
    if (!projectileDef) {
      gameLocal.Warning("Unknown projectile '%s' in weapon '%s'",
                        projectileName.c_str(), objectname.c_str());
    } else {
      const std::string& spawnclass =
          projectileDef->dict.GetString("spawnclass");
      idTypeInfo* cls = idClass::GetClass(spawnclass);
      if (!cls || !cls->IsType(idProjectile::Type)) {
        gameLocal.Warning(
            "Invalid spawnclass '%s' on projectile '%s' (used by weapon '%s')",
            spawnclass.c_str(), projectileName.c_str(), objectname.c_str());
      }
    }
  }
}

void idProjectile::Damage(idEntity* inflictor, idEntity* attacker,
                          const Vector2& dir,
                          const std::string& damageDefName) {
  if (!fl.takedamage) {
    return;
  }

  if (!inflictor) {
    inflictor = gameLocal.world.get();
  }

  if (!attacker) {
    attacker = gameLocal.world.get();
  }

  const idDict* damageDef = gameLocal.FindEntityDefDict(damageDefName);
  if (damageDef == NULL) {
    gameLocal.Error("Unknown damageDef '%s'\n", damageDefName.c_str());
    return;
  }

  int damage = damageDef->GetInt("damage");

  if (g_debugDamage.GetInteger()) {
    gameLocal.Printf(
        "inflictor:%02d\ttarget:%02d\tdamage "
        "type:%s\t\thealth:%03d\tdamage:%03d\n",
        inflictor->entityNumber, entityNumber, damageDefName.c_str(), health,
        damage);
  }

  // inform the attacker that they hit someone
  // attacker->DamageFeedback(this, inflictor, damage);
  if (damage) {
    // do the damage
    health -= damage;
    if (health <= 0) {
      if (health < -999) {
        health = -999;
      }

      PostEventSec(&EV_Explode, 0);
    } else {
      // Pain(inflictor, attacker, damage, dir, location);
    }
  }
}

/*
================
idProjectile::ClientThink
================
*/
void idProjectile::ClientThink(
    /*const int curTime, const float fraction, const bool predict*/) {
  Present();
}

/*
================
idProjectile::WriteToSnapshot
================
*/
void idProjectile::WriteToSnapshot(idBitMsg& msg) const {
  msg.WriteLong(owner.GetSpawnId());
  msg.WriteBytes(static_cast<int>(state), 1);
  msg.WriteBytes(fl.hidden, 1);

  physicsObj->WriteToSnapshot(msg);
}

/*
================
idProjectile::ReadFromSnapshot
================
*/
void idProjectile::ReadFromSnapshot(const idBitMsg& msg) {
  projectileState_t newState;

  owner.SetSpawnId(msg.ReadLong());
  newState = static_cast<projectileState_t>(msg.ReadBytes(1));

  if (msg.ReadBytes(1)) {
    Hide();
  } else {
    Show();
  }

  while (state != newState) {
    switch (state) {
      case projectileState_t::SPAWNED: {
        Create(owner.GetEntity(), vec2_origin, vec2_origin);
        break;
      }
      case projectileState_t::CREATED: {
        // the right origin and direction are required if you want bullet traces
        Launch(vec2_origin, vec2_origin, vec2_origin);
        break;
      }
      case projectileState_t::LAUNCHED: {
        if (newState == projectileState_t::FIZZLED) {
          // Fizzle();
        } else {
          trace_t collision;
          memset(&collision, 0, sizeof(collision));
          collision.endpos = GetPhysics()->GetOrigin();
          Explode(collision, NULL);
        }
        break;
      }
      case projectileState_t::FIZZLED:
      case projectileState_t::EXPLODED: {
        gameEdit->ParseSpawnArgsToRenderEntity(&spawnArgs, &renderEntity);
        state = projectileState_t::SPAWNED;
        break;
      }
    }
  }

  physicsObj->ReadFromSnapshot(msg);

  if (true /*msg.HasChanged()*/) {
    UpdateVisuals();
  }
}