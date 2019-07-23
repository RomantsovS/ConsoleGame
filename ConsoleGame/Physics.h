#ifndef PHYSICS_H
#define PHYSICS_H

#include "Class.h"
#include "Vector2.h"

class idEntity;

class idPhysics : public idClass
{
public:
	ABSTRACT_PROTOTYPE(idPhysics);

	enum directions { LEFT, RIGHT, UP, DOWN };

	virtual ~idPhysics() = 0;

	// set pointer to entity using physics
	virtual void SetSelf(std::shared_ptr<idEntity> e) = 0;

	// evaluate the physics with the given time step, returns true if the object moved
	virtual bool Evaluate(int timeStepMSec, int endTimeMSec) = 0;

	// update the time without moving
	virtual void UpdateTime(int endTimeMSec) = 0;
	// get the last physics update time
	virtual int	GetTime() const = 0;

	virtual void Activate() = 0;

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

	/*virtual bool checkCollide(const Game &Game) const = 0;
	virtual bool checkCollide(std::shared_ptr<Point> point) = 0;
	virtual bool checkCollide(const pos_type pos) const = 0;*/
};

#endif

