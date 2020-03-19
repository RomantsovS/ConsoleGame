#ifndef PHYSICS_BASE_H
#define PHYSICS_BASE_H

#include "Physics.h"

class idPhysics_Base : public idPhysics
{
public:
	CLASS_PROTOTYPE(idPhysics_Base);

	idPhysics_Base();

	~idPhysics_Base();

	void SetSelf(std::shared_ptr<idEntity> e) override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) override;

	void UpdateTime(int endTimeMSec) override;
	int	GetTime() const override;

	void Activate() override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) override;

	const Vector2 & GetOrigin(int id = 0) const override;
	const Vector2 &	GetAxis(int id = 0) const override;
protected:
protected:
	std::shared_ptr<idEntity> self; // entity using this physics object
};

#endif