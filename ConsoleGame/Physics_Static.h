#ifndef PHYSICS_STATIC_H
#define PHYSICS_STATIC_H

#include "Physics.h"

class Entity;

struct staticPState_s
{
	Vector2 origin;
	Vector2	axis;
	Vector2 localOrigin;
	Vector2	localAxis;
};

class Physics_Static : public Physics
{
public:
	Physics_Static();
	
	~Physics_Static();

	void SetSelf(Entity *e) override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) override;

	void UpdateTime(int endTimeMSec) override;
	int	GetTime() const override;

	void Activate() override;

	void SaveState();
	void RestoreState();

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) override;

	const Vector2 & GetOrigin(int id = 0) const override;
	const Vector2 &	GetAxis(int id = 0) const override;
protected:
protected:
	Entity * self;					// entity using this physics object
	staticPState_s current;			// physics state
};

#endif