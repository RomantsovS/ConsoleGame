#ifndef PHYSICS_H
#define PHYSICS_H

extern const float CONTACT_EPSILON;

class idEntity;

class idPhysics : public idClass {
 public:
  CLASS_PROTOTYPE(idPhysics);

  enum directions { LEFT, RIGHT, UP, DOWN };

  idPhysics() = default;
  virtual ~idPhysics() = 0;
  idPhysics(const idPhysics&) = default;
  idPhysics& operator=(const idPhysics&) = default;
  idPhysics(idPhysics&&) = default;
  idPhysics& operator=(idPhysics&&) = default;

 public:  // common physics interface
  // set pointer to entity using physics
  virtual void SetSelf(idEntity* e) = 0;
  // clip models
  virtual void SetClipModel(std::shared_ptr<idClipModel> model, float density,
                            int id = 0, bool freeOld = true) = 0;
  virtual std::shared_ptr<idClipModel> GetClipModel(int id = 0) const = 0;
  virtual int GetNumClipModels() const = 0;
  // get/set the contents of a specific clip model or the whole physics object
  virtual void SetContents(int contents, int id = -1) = 0;
  virtual int GetContents(int id = -1) const = 0;
  // get/set the contents a specific clip model or the whole physics object
  // collides with
  virtual void SetClipMask(int mask, int id = -1) = 0;
  virtual int GetClipMask(int id = -1) const = 0;
  // get the bounds of a specific clip model or the whole physics object
  virtual const idBounds& GetBounds(int id = -1) const = 0;
  virtual const idBounds& GetAbsBounds(int id = -1) const = 0;

  // evaluate the physics with the given time step, returns true if the object
  // moved
  virtual bool Evaluate(int timeStepMSec, int endTimeMSec) = 0;

  // update the time without moving
  virtual void UpdateTime(int endTimeMSec) = 0;
  // get the last physics update time
  virtual int GetTime() const = 0;

  virtual void Activate() = 0;
  virtual void PutToRest() = 0;
  virtual bool IsAtRest() const = 0;

  // save and restore the physics state
  virtual void SaveState() = 0;
  virtual void RestoreState() = 0;

  // set the position and orientation in master space or world space if no
  // master set
  virtual void SetOrigin(const Vector2& newOrigin, int id = -1) = 0;
  virtual void SetAxis(const Vector2& newAxis, int id = -1) = 0;
  // translate or rotate the physics object in world space
  virtual void Translate(const Vector2& translation, int id = -1) = 0;
  virtual void Rotate(const Vector2& rotation, int id = -1) = 0;
  // get the position and orientation in world space
  virtual const Vector2& GetOrigin(int id = 0) const = 0;
  virtual const Vector2& GetAxis(int id = 0) const = 0;
  // set linear and angular velocity
  virtual void SetLinearVelocity(const Vector2& newLinearVelocity,
                                 int id = 0) = 0;
  // get linear and angular velocity
  virtual const Vector2& GetLinearVelocity(int id = 0) const = 0;

  virtual void DisableClip() = 0;
  virtual void EnableClip() = 0;
  // link/unlink the clip models contained by this physics object
  virtual void UnlinkClip() = 0;
  virtual void LinkClip() = 0;
  // contacts
  virtual bool EvaluateContacts() = 0;

  virtual void ClearContacts() = 0;
  virtual void AddContactEntity(idEntity* e) = 0;
  virtual void RemoveContactEntity(idEntity* e) = 0;

  // networking
  virtual void WriteToSnapshot(idBitMsg& msg) const = 0;
  virtual void ReadFromSnapshot(const idBitMsg& msg) = 0;
};

#endif
