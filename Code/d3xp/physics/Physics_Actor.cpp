
#include "idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_Base, idPhysics_Actor)
END_CLASS

/*
================
idPhysics_Actor::idPhysics_Actor
================
*/
idPhysics_Actor::idPhysics_Actor() {
  clipModel = nullptr;
  SetClipModelAxis();
}

/*
================
idPhysics_Actor::~idPhysics_Actor
================
*/
idPhysics_Actor::~idPhysics_Actor() {
  if (clipModel) {
    clipModel = nullptr;
  }
}

/*
================
idPhysics_Actor::SetClipModelAxis
================
*/
void idPhysics_Actor::SetClipModelAxis() {
  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, clipModel->GetOrigin());
  }
}

/*
================
idPhysics_Actor::SetClipModel
================
*/
void idPhysics_Actor::SetClipModel(std::shared_ptr<idClipModel> model,
                                   const float density, int id,
                                   bool freeOld) noexcept {
  if (clipModel && clipModel != model && freeOld) {
    clipModel = nullptr;
  }
  clipModel = model;
  clipModel->Link(gameLocal.clip, self, 0, clipModel->GetOrigin());
}

/*
================
idPhysics_Actor::GetClipModel
================
*/
std::shared_ptr<idClipModel> idPhysics_Actor::GetClipModel(
    int id) const noexcept {
  return clipModel;
}

/*
================
idPhysics_Actor::GetNumClipModels
================
*/
int idPhysics_Actor::GetNumClipModels() const noexcept { return 1; }

/*
================
idPhysics_Actor::SetClipMask
================
*/
void idPhysics_Actor::SetContents(int contents, int id) {
  clipModel->SetContents(contents);
}

/*
================
idPhysics_Actor::SetClipMask
================
*/
int idPhysics_Actor::GetContents(int id) const {
  return clipModel->GetContents();
}

/*
================
idPhysics_Actor::GetBounds
================
*/
const idBounds& idPhysics_Actor::GetBounds(int id) const noexcept {
  return clipModel->GetBounds();
}

/*
================
idPhysics_Actor::GetAbsBounds
================
*/
const idBounds& idPhysics_Actor::GetAbsBounds(int id) const noexcept {
  return clipModel->GetAbsBounds();
}

/*
================
idPhysics_Actor::GetOrigin
================
*/
const Vector2& idPhysics_Actor::GetOrigin(int id) const noexcept {
  return clipModel->GetOrigin();
}

/*
================
idPhysics_Actor::DisableClip
================
*/
void idPhysics_Actor::DisableClip() noexcept { clipModel->Disable(); }

/*
================
idPhysics_Actor::EnableClip
================
*/
void idPhysics_Actor::EnableClip() noexcept { clipModel->Enable(); }

/*
================
idPhysics_Actor::UnlinkClip
================
*/
void idPhysics_Actor::UnlinkClip() noexcept { clipModel->Unlink(); }

/*
================
idPhysics_Actor::LinkClip
================
*/
void idPhysics_Actor::LinkClip() noexcept {
  clipModel->Link(gameLocal.clip, self, 0, clipModel->GetOrigin());
}

/*
================
idPhysics_Actor::EvaluateContacts
================
*/
bool idPhysics_Actor::EvaluateContacts() noexcept {
  // get all the ground contacts
  ClearContacts();
  AddGroundContacts(clipModel.get());
  AddContactEntitiesForContacts();

  return (!contacts.empty());
}