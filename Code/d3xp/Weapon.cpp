#include "idlib/precompiled.h"

#include "Game_local.h"

const idEventDef EV_Weapon_WeaponReady("weaponReady");

CLASS_DECLARATION(idAnimatedEntity, idWeapon)
EVENT(EV_Weapon_WeaponReady, idWeapon::Event_WeaponReady)
END_CLASS

/*
================
idWeapon::idWeapon()
================
*/
idWeapon::idWeapon() { Clear(); }

/*
================
idWeapon::~idWeapon()
================
*/
idWeapon::~idWeapon() { Clear(); }

/*
================
idWeapon::Spawn
================
*/
void idWeapon::Spawn() noexcept {}

/*
================
idWeapon::SetOwner

Only called at player spawn time, not each weapon switch
================
*/
void idWeapon::SetOwner(idPlayer* _owner) {
  idassert(owner);
  owner = _owner;
  SetName(va("%s_weap", _owner->name.c_str()));
}

/*
================
idWeapon::CacheWeapon
================
*/
void idWeapon::CacheWeapon(const std::string& weaponName) {
  std::shared_ptr<idDeclEntityDef> weaponDef =
      gameLocal.FindEntityDef(weaponName, false);
  if (!weaponDef) {
    return;
  }
}

/*
================
idWeapon::Clear
================
*/
void idWeapon::Clear() noexcept {
  status = weaponStatus_t::WP_HOLSTERED;

  projectileDict.Clear();
}

/*
================
idWeapon::GetWeaponDef
================
*/
void idWeapon::GetWeaponDef(const std::string& objectname, int ammoinclip) {
  Clear();

  if (objectname.empty()) {
    return;
  }

  idassert(owner);

  weaponDef = gameLocal.FindEntityDef(objectname).get();

  // get the projectile
  projectileDict.Clear();

  std::string projectileName = weaponDef->dict.GetString("def_projectile");
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
      } else {
        projectileDict = projectileDef->dict;
      }
    }

    idProjectile::CacheProjectille(projectileName, projectileDef.get());
  }

  spawnArgs = weaponDef->dict;
}

/*
================
idWeapon::Think
================
*/
void idWeapon::Think() noexcept {
  // do nothing because the present is called from the player through
  // PresentWeapon
}

/*
================
idWeapon::BeginAttack
================
*/
void idWeapon::BeginAttack() noexcept {
  if (status != weaponStatus_t::WP_OUTOFAMMO) {
    lastAttack = gameLocal.time;
  }

  status = weaponStatus_t::WP_FIRING;
}

/*
================
idWeapon::EndAttack
================
*/
void idWeapon::EndAttack() noexcept {
  if (WEAPON_ATTACK) {
    WEAPON_ATTACK = false;
  }
}

/*
================
idWeapon::isReady
================
*/
bool idWeapon::IsReady() const noexcept {
  return !IsHidden() && ((status == weaponStatus_t::WP_RELOAD) ||
                         (status == weaponStatus_t::WP_READY) ||
                         (status == weaponStatus_t::WP_OUTOFAMMO));
}

/*
================
idWeapon::IsHolstered
================
*/
bool idWeapon::IsHolstered() const noexcept {
  return (status == weaponStatus_t::WP_HOLSTERED);
}

/*
================
idWeapon::UpdateScript
================
*/
void idWeapon::UpdateScript() {
  std::shared_ptr<idEntity> ent;

  if (status == weaponStatus_t::WP_FIRING) {
    gameLocal.SpawnEntityDef(projectileDict, &ent);

    if (!ent || !ent->IsType(idProjectile::Type)) {
      const std::string projectileName =
          weaponDef->dict.GetString("def_projectile");
      gameLocal.Error("'%s' is not an idProjectile", projectileName.c_str());
      return;
    }

    std::shared_ptr<idProjectile> proj =
        std::static_pointer_cast<idProjectile>(ent);
    proj->Create(owner, owner->GetPhysics()->GetOrigin(), vec2_origin);

    Vector2 pos = owner->GetPhysics()->GetOrigin();
    pos.x = 8 + static_cast<int>(pos.x) / 16 * 16;
    pos.y = 8 + static_cast<int>(pos.y) / 16 * 16;
    proj->Launch(pos, owner->GetPhysics()->GetLinearVelocity(), vec2_origin);

    PostEventMS(&EV_Weapon_WeaponReady, spawnArgs.GetInt("ready_time", "500"));

    status = weaponStatus_t::WP_WAITING;
  }
}

/*
================
idWeapon::PresentWeapon
================
*/
void idWeapon::PresentWeapon() {
  // update the weapon script
  UpdateScript();
}

/*
===============
idWeapon::Event_WeaponReady
===============
*/
void idWeapon::Event_WeaponReady() noexcept {
  status = weaponStatus_t::WP_READY;
}