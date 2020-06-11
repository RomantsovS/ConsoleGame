#ifndef PHYSICS_H
#define PHYSICS_H

#include "../gamesys/Class.h"
#include "../../idlib/math/Vector2.h"
#include "Clip.h"

const float CONTACT_EPSILON = 0.25f; // maximum contact seperation distance

class idEntity;

class idPhysics : public idClass
{
public:
	ABSTRACT_PROTOTYPE(idPhysics);

	enum directions { LEFT, RIGHT, UP, DOWN };

	virtual ~idPhysics() = 0;
public: // common physics interface
	// set pointer to entity using physics
	virtual void SetSelf(std::shared_ptr<idEntity> e) = 0;
								// clip models
	virtual void SetClipModel( std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true ) = 0;
	virtual std::shared_ptr<idClipModel> GetClipModel(int id = 0) const = 0;
	virtual int GetNumClipModels() const = 0;
	// get/set the contents a specific clip model or the whole physics object collides with
	virtual void SetClipMask(int mask, int id = -1) = 0;
	virtual int GetClipMask(int id = -1) const = 0;
	// get the bounds of a specific clip model or the whole physics object
	virtual const idBounds& GetBounds(int id = -1) const = 0;
	virtual const idBounds& GetAbsBounds(int id = -1) const = 0;

	// evaluate the physics with the given time step, returns true if the object moved
	virtual bool Evaluate(int timeStepMSec, int endTimeMSec) = 0;

	// update the time without moving
	virtual void UpdateTime(int endTimeMSec) = 0;
	// get the last physics update time
	virtual int	GetTime() const = 0;

	virtual void Activate() = 0;
	virtual void PutToRest() = 0;
	virtual bool IsAtRest() const = 0;

	// save and restore the physics state
	virtual void SaveState() = 0;
	virtual void RestoreState() = 0;

	// set the position and orientation in master space or world space if no master set
	virtual void SetOrigin(const Vector2 &newOrigin, int id = -1) = 0;
	virtual void SetAxis(const Vector2 &newAxis, int id = -1) = 0;
	// translate or rotate the physics object in world space
	virtual void Translate(const Vector2 &translation, int id = -1) = 0;
	virtual void Rotate(const Vector2 &rotation, int id = -1) = 0;
	// get the position and orientation in world space
	virtual const Vector2 &	GetOrigin(int id = 0) const = 0;
	virtual const Vector2 &	GetAxis(int id = 0) const = 0;
	// set linear and angular velocity
	virtual void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) = 0;
	// get linear and angular velocity
	virtual const Vector2& GetLinearVelocity(int id = 0) const = 0;

	virtual void ClearContacts() = 0;
	virtual void AddContactEntity(std::shared_ptr<idEntity> e) = 0;
	virtual void RemoveContactEntity(std::shared_ptr<idEntity> e) = 0;

	/*virtual bool checkCollide(const Game &Game) const = 0;
	virtual bool checkCollide(std::shared_ptr<Point> point) = 0;
	virtual bool checkCollide(const pos_type pos) const = 0;*/
};

#endif

