#include "idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics, idPhysics_Static)
END_CLASS

idPhysics_Static::idPhysics_Static() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "idPhysics_Static");
#endif  // DEBUG_PRINT_Ctor_Dtor
}

idPhysics_Static::~idPhysics_Static() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s dtor\n", "idPhysics_Static");
#endif  // DEBUG_PRINT_Ctor_Dtor

  /*if (!self.expired() && self.lock()->GetPhysics().get() == this)
  {
          self.reset();
  }*/
}

void idPhysics_Static::SetSelf(idEntity* e) noexcept { self = e; }

void idPhysics_Static::SetClipModel(std::shared_ptr<idClipModel> model,
                                    float density, int id, bool freeOld) {
  if (clipModel && clipModel != model && freeOld) {
    clipModel = nullptr;
  }
  clipModel = model;
  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, current.origin);
  }
}

/*
================
idPhysics_Static::GetClipModel
================
*/
std::shared_ptr<idClipModel> idPhysics_Static::GetClipModel(
    int id) const noexcept {
  if (clipModel) {
    return clipModel;
  }
  return gameLocal.clip.DefaultClipModel();
}

/*
================
idPhysics_Static::GetNumClipModels
================
*/
int idPhysics_Static::GetNumClipModels() const noexcept {
  return (clipModel != nullptr);
}

/*
================
idPhysics_Static::SetContents
================
*/
void idPhysics_Static::SetContents(int contents, int id) {
  if (clipModel) {
    clipModel->SetContents(contents);
  }
}

/*
================
idPhysics_Static::GetContents
================
*/
int idPhysics_Static::GetContents(int id) const {
  if (clipModel) {
    return clipModel->GetContents();
  }
  return 0;
}

/*
================
idPhysics_Static::SetClipMask
================
*/
void idPhysics_Static::SetClipMask(int mask, int id) noexcept {}

/*
================
idPhysics_Static::GetClipMask
================
*/
int idPhysics_Static::GetClipMask(int id) const noexcept { return 0; }

/*
================
idPhysics_Static::GetBounds
================
*/
const idBounds& idPhysics_Static::GetBounds(int id) const noexcept {
  if (clipModel) {
    return clipModel->GetBounds();
  }
  return bounds_zero;
}

/*
================
idPhysics_Static::GetAbsBounds
================
*/
const idBounds& idPhysics_Static::GetAbsBounds(int id) const {
  static idBounds absBounds;

  if (clipModel) {
    return clipModel->GetAbsBounds();
  }
  absBounds[0] = absBounds[1] = current.origin;
  return absBounds;
}

bool idPhysics_Static::Evaluate(int timeStepMSec, int endTimeMSec) noexcept {
  // Vector2 masterOrigin, oldOrigin;

  /*if (hasMaster) {
          oldOrigin = current.origin;
          oldAxis = current.axis;

          self->GetMasterPosition(masterOrigin, masterAxis);
          current.origin = masterOrigin + current.localOrigin * masterAxis;
          if (isOrientated) {
                  current.axis = current.localAxis * masterAxis;
          }
          else {
                  current.axis = current.localAxis;
          }
          if (clipModel) {
                  clipModel->Link(gameLocal.clip, self, 0, current.origin,
  current.axis);
          }

          return (current.origin != oldOrigin || current.axis != oldAxis);
  }*/
  return false;
}

void idPhysics_Static::UpdateTime(int endTimeMSec) noexcept {}

int idPhysics_Static::GetTime() const noexcept { return 0; }

void idPhysics_Static::Activate() noexcept {}

/*
================
idPhysics_Static::PutToRest
================
*/
void idPhysics_Static::PutToRest() noexcept {}

bool idPhysics_Static::IsAtRest() const noexcept { return true; }

void idPhysics_Static::SaveState() noexcept {}

void idPhysics_Static::RestoreState() noexcept {}

void idPhysics_Static::SetOrigin(const Vector2& newOrigin, int id) {
  /*Vector2 masterOrigin;
  Vector2 masterAxis;*/

  current.localOrigin = newOrigin;

  /*if (hasMaster) {
          self->GetMasterPosition(masterOrigin, masterAxis);
          current.origin = masterOrigin + newOrigin * masterAxis;
  }
  else {*/
  current.origin = newOrigin;
  //}

  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, current.origin);
  }

  /*next = ConvertPStateToInterpolateState(current);
  previous = next;*/
}

void idPhysics_Static::SetAxis(const Vector2& newAxis, int id) noexcept {
  /*Vector2 masterOrigin;
  Vector2 masterAxis;*/

  current.localAxis = newAxis;

  /*if (hasMaster && isOrientated) {
          self->GetMasterPosition(masterOrigin, masterAxis);
          current.axis = newAxis * masterAxis;
  }
  else {*/
  current.axis = newAxis;
  /*}

  if (clipModel) {
          clipModel->Link(gameLocal.clip, self, 0, current.origin,
  current.axis);
  }

  next = ConvertPStateToInterpolateState(current);
  previous = next;*/
}

void idPhysics_Static::Translate(const Vector2& translation, int id) {
  current.localOrigin += translation;
  current.origin += translation;

  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, current.origin);
  }
}

void idPhysics_Static::Rotate(const Vector2& rotation, int id) noexcept {
  /*Vector2 masterOrigin;
  Vector2 masterAxis;*/

  // current.origin *= rotation;
  // current.axis *= rotation.ToMat3();
  current.origin = rotation;
  current.axis = rotation;

  /*if (hasMaster) {
          self->GetMasterPosition(masterOrigin, masterAxis);
          current.localAxis *= rotation.ToMat3();
          current.localOrigin = (current.origin - masterOrigin) *
  masterAxis.Transpose();
  }
  else {*/
  current.localAxis = current.axis;
  current.localOrigin = current.origin;
  /*}

  if (clipModel) {
          clipModel->Link(gameLocal.clip, self, 0, current.origin,
  current.axis);
  }*/
}

const Vector2& idPhysics_Static::GetOrigin(int id) const noexcept {
  return current.origin;
}

const Vector2& idPhysics_Static::GetAxis(int id) const noexcept {
  return current.axis;
}

void idPhysics_Static::SetLinearVelocity(const Vector2& newLinearVelocity,
                                         int id) noexcept {}

/*
================
idPhysics_Static::GetLinearVelocity
================
*/
const Vector2& idPhysics_Static::GetLinearVelocity(int id) const noexcept {
  return vec2_origin;
}

/*
================
idPhysics_Static::DisableClip
================
*/
void idPhysics_Static::DisableClip() noexcept {
  if (clipModel) {
    clipModel->Disable();
  }
}

/*
================
idPhysics_Static::EnableClip
================
*/
void idPhysics_Static::EnableClip() noexcept {
  if (clipModel) {
    clipModel->Enable();
  }
}

/*
================
idPhysics_Static::UnlinkClip
================
*/
void idPhysics_Static::UnlinkClip() noexcept {
  if (clipModel) {
    clipModel->Unlink();
  }
}

/*
================
idPhysics_Static::LinkClip
================
*/
void idPhysics_Static::LinkClip() {
  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, current.origin);
  }
}

/*
================
idPhysics_Static::EvaluateContacts
================
*/
bool idPhysics_Static::EvaluateContacts() noexcept { return false; }

void idPhysics_Static::ClearContacts() noexcept {}

void idPhysics_Static::AddContactEntity(idEntity* e) noexcept {}

void idPhysics_Static::RemoveContactEntity(idEntity* e) noexcept {}

/*
================
idPhysics_Static::WriteToSnapshot
================
*/
void idPhysics_Static::WriteToSnapshot(idBitMsg& msg) const {
  msg.WriteFloat(current.origin[0]);
  msg.WriteFloat(current.origin[1]);
}

/*
================
idPhysics_Base::ReadFromSnapshot
================
*/
void idPhysics_Static::ReadFromSnapshot(const idBitMsg& msg) {
  current.origin[0] = msg.ReadFloat();
  current.origin[1] = msg.ReadFloat();
}