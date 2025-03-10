#include "idlib/precompiled.h"

#include "Game_local.h"

CLASS_DECLARATION(idEntity, idPlayerStart)
END_CLASS

void idPlayerStart::Spawn() {}

CLASS_DECLARATION(idEntity, idStaticEntity)
END_CLASS

idStaticEntity::idStaticEntity() {
  // spawnTime = 0;
}

idStaticEntity::~idStaticEntity() {}

void idStaticEntity::Spawn() {
  GetPhysics()->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_BODY));
  fl.takedamage = !spawnArgs.GetBool("noDamage");
}

void idStaticEntity::Think() { idEntity::Think(); }

void idStaticEntity::Killed(idEntity* inflictor, idEntity* attacker, int damage,
                            const Vector2& dir) noexcept {
  Hide();
  PostEventMS(&EV_Remove, 100);
}

/*
================
idStaticEntity::WriteToSnapshot
================
*/
void idStaticEntity::WriteToSnapshot(idBitMsg& msg) const {
  msg.WriteBytes(fl.hidden, 1);
  GetPhysics()->WriteToSnapshot(msg);
}

/*
================
idStaticEntity::ReadFromSnapshot
================
*/
void idStaticEntity::ReadFromSnapshot(const idBitMsg& msg) {
  if (msg.ReadBytes(1)) {
    Hide();
  } else {
    Show();
  }

  GetPhysics()->ReadFromSnapshot(msg);

  if (true /*msg.HasChanged()*/) {
    UpdateVisuals();
  }
}

CLASS_DECLARATION(idAnimatedEntity, idSimpleObject)
END_CLASS

idSimpleObject::idSimpleObject() {}

idSimpleObject::~idSimpleObject() {}

void idSimpleObject::Spawn() {
  idTraceModel trm;
  float density;
  std::string clipModelName;

  // check if a clip model is set
  /*spawnArgs.GetString("clipmodel", "", &clipModelName);
  if (!clipModelName[0]) {
          clipModelName = spawnArgs.GetString("model");		// use the
  visual model
  }

  if (!collisionModelManager->TrmFromModel(clipModelName, trm)) {
          gameLocal.Error("idSimpleObject '%s': cannot load collision model %s",
  name, clipModelName); return;
  }*/

  density = 0.0f;

  physicsObj = std::make_shared<idPhysics_RigidBody>();
  physicsObj->SetSelf(this);
  // physicsObj->SetClipModel(std::make_shared<idClipModel>(trm), density);
  physicsObj->SetClipModel(
      std::make_shared<idClipModel>(*GetPhysics()->GetClipModel()), density);
  physicsObj->SetOrigin(GetPhysics()->GetOrigin());

  SetPhysics(physicsObj);

  Vector2 linearVelocity;
  spawnArgs.GetVector("linearVelocity", "0 0", linearVelocity);
  physicsObj->SetLinearVelocity(linearVelocity);
}

void idSimpleObject::Think() { idEntity::Think(); }

void idSimpleObject::Remove() noexcept {
  physicsObj->SetClipModel(nullptr, 0.0f);

  physicsObj = nullptr;
  idEntity::Remove();
}

bool idSimpleObject::Collide(const trace_t& collision,
                             const Vector2& velocity) noexcept {
  if (collision.c.entityNum == ENTITYNUM_WORLD) {
    return true;
  } else {
    auto ent = gameLocal.entities[collision.c.entityNum].get();

    if (!ent->IsActive()) return true;
  }

  return false;
}
