#ifndef PHYSICS_RIDIGBODY_H
#define PHYSICS_RIDIGBODY_H

#include "Physics_Base.h"

class idPhysics_RigidBody : public idPhysics_Base
{
public:
	CLASS_PROTOTYPE(idPhysics_RigidBody);

	idPhysics_RigidBody();
	~idPhysics_RigidBody();

public:	// common physics interface
	bool Evaluate(int timeStepMSec, int endTimeMSec) override;
	void UpdateTime(int endTimeMSec) override;
	int GetTime() const override;

	void Activate() override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) override;

	const Vector2 & GetOrigin(int id = 0) const override;
	const Vector2 &	GetAxis(int id = 0) const override;
private:
	// state of the rigid body
	rigidBodyPState_t current;
	rigidBodyPState_t saved;
};

#endif