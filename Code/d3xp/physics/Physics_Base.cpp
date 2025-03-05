#include "idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics, idPhysics_Base)
END_CLASS

idPhysics_Base::idPhysics_Base() {
  self = nullptr;
  clipMask = 0;
  ClearContacts();
}

idPhysics_Base::~idPhysics_Base() {}

void idPhysics_Base::SetSelf(idEntity* e) noexcept { self = e; }

void idPhysics_Base::SetClipModel(std::shared_ptr<idClipModel> model,
                                  float density, int id,
                                  bool freeOld) noexcept {}

std::shared_ptr<idClipModel> idPhysics_Base::GetClipModel(
    int id) const noexcept {
  return nullptr;
}

int idPhysics_Base::GetNumClipModels() const noexcept { return 0; }

/*
================
idPhysics_Base::SetContents
================
*/
void idPhysics_Base::SetContents(int contents, int id) {}

/*
================
idPhysics_Base::SetClipMask
================
*/
int idPhysics_Base::GetContents(int id) const { return 0; }

/*
================
idPhysics_Base::SetClipMask
================
*/
void idPhysics_Base::SetClipMask(int mask, int id) noexcept { clipMask = mask; }

/*
================
idPhysics_Base::GetClipMask
================
*/
int idPhysics_Base::GetClipMask(int id) const noexcept { return clipMask; }

/*
================
idPhysics_Base::GetBounds
================
*/
const idBounds& idPhysics_Base::GetBounds(int id) const noexcept {
  return bounds_zero;
}

/*
================
idPhysics_Base::GetAbsBounds
================
*/
const idBounds& idPhysics_Base::GetAbsBounds(int id) const noexcept {
  return bounds_zero;
}

bool idPhysics_Base::Evaluate(int timeStepMSec, int endTimeMSec) noexcept {
  return false;
}

void idPhysics_Base::UpdateTime(int endTimeMSec) noexcept {}

int idPhysics_Base::GetTime() const noexcept { return 0; }

void idPhysics_Base::Activate() noexcept {}

/*
================
idPhysics_Base::PutToRest
================
*/
void idPhysics_Base::PutToRest() noexcept {}

bool idPhysics_Base::IsAtRest() const noexcept { return true; }

void idPhysics_Base::SaveState() noexcept {}

void idPhysics_Base::RestoreState() noexcept {}

void idPhysics_Base::SetOrigin(const Vector2& newOrigin, int id) noexcept {}

void idPhysics_Base::SetAxis(const Vector2& newAxis, int id) noexcept {}

void idPhysics_Base::Translate(const Vector2& translation, int id) noexcept {}

void idPhysics_Base::Rotate(const Vector2& rotation, int id) noexcept {}

const Vector2& idPhysics_Base::GetOrigin(int id) const noexcept {
  return vec2_origin;
}

const Vector2& idPhysics_Base::GetAxis(int id) const noexcept {
  return vec2_origin;
}

void idPhysics_Base::SetLinearVelocity(const Vector2& newLinearVelocity,
                                       int id) noexcept {}

/*
================
idPhysics_Base::GetLinearVelocity
================
*/
const Vector2& idPhysics_Base::GetLinearVelocity(int id) const noexcept {
  return vec2_origin;
}

/*
================
idPhysics_Base::DisableClip
================
*/
void idPhysics_Base::DisableClip() noexcept {}

/*
================
idPhysics_Base::EnableClip
================
*/
void idPhysics_Base::EnableClip() noexcept {}

/*
================
idPhysics_Base::UnlinkClip
================
*/
void idPhysics_Base::UnlinkClip() noexcept {}

/*
================
idPhysics_Base::LinkClip
================
*/
void idPhysics_Base::LinkClip() noexcept {}

/*
================
idPhysics_Base::EvaluateContacts
================
*/
bool idPhysics_Base::EvaluateContacts() noexcept { return false; }

/*
================
idPhysics_Base::GetNumContacts
================
*/
int idPhysics_Base::GetNumContacts() const { return contacts.size(); }

/*
================
idPhysics_Base::GetContact
================
*/
const contactInfo_t& idPhysics_Base::GetContact(int num) const {
  return contacts[num];
}

void idPhysics_Base::ClearContacts() {
  for (auto iter = contacts.begin(); iter != contacts.end(); ++iter) {
    auto& ent = gameLocal.entities[iter->entityNum];
    if (ent) {
      ent->RemoveContactEntity(self);
    }
  }
  contacts.clear();
}

void idPhysics_Base::AddContactEntity(idEntity* e) {
  bool found = false;

  for (auto iter = contactEntities.begin(); iter != contactEntities.end();
       ++iter) {
    auto ent = *iter;
    if (!ent) {
      iter = contactEntities.erase(iter);
    }
    if (ent == e) {
      found = true;
    }
  }
  if (!found) {
    contactEntities.push_back(e);
  }
}

void idPhysics_Base::RemoveContactEntity(idEntity* e) noexcept {
  /*for (auto iter = contactEntities.begin(); iter != contactEntities.end();
  ++iter) { auto ent = *iter; if (!ent) { iter = contactEntities.erase(iter);
                  continue;
          }
          if (ent == e) {
                  iter = contactEntities.erase(iter);
                  return;
          }
  }*/
  contactEntities.erase(
      std::remove_if(contactEntities.begin(), contactEntities.end(),
                     [e](idEntity* ent) { return !ent || ent == e; }),
      contactEntities.end());
}

/*
================
idPhysics_Base::AddGroundContacts
================
*/
void idPhysics_Base::AddGroundContacts(const idClipModel* clipModel) {
  Vector2 dir;
  size_t index, num;

  index = contacts.size();
  contacts.resize(index + 10);

  dir = vec2_point_size;
  num =
      gameLocal.clip.Contacts(&contacts[index], 10, clipModel->GetOrigin(), dir,
                              CONTACT_EPSILON, clipModel, clipMask, self);
  contacts.resize(index + num);
}

void idPhysics_Base::AddContactEntitiesForContacts() {
  for (size_t i = 0; i < contacts.size(); i++) {
    auto& ent = gameLocal.entities[contacts[i].entityNum];
    if (ent && ent.get() != self) {
      ent->AddContactEntity(self);
    }
  }
}

void idPhysics_Base::ActivateContactEntities() {
  for (auto iter = contactEntities.begin(); iter != contactEntities.end();
       ++iter) {
    auto ent = *iter;
    if (ent) {
      ent->ActivatePhysics(self);
    } else {
      iter = contactEntities.erase(iter);
    }
  }
}

bool idPhysics_Base::IsOutsideWorld() const {
  if (!gameLocal.clip.GetWorldBounds()
           .Expand(CM_CLIP_EPSILON)
           .IntersectsBounds(GetAbsBounds())) {
    return true;
  }
  return false;
}

/*
================
idPhysics_Base::WriteToSnapshot
================
*/
void idPhysics_Base::WriteToSnapshot(idBitMsg& msg) const {}

/*
================
idPhysics_Base::ReadFromSnapshot
================
*/
void idPhysics_Base::ReadFromSnapshot(const idBitMsg& msg) {}