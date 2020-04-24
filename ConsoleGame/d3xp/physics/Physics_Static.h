#ifndef PHYSICS_STATIC_H
#define PHYSICS_STATIC_H

#include "Physics.h"
#include "Clip.h"

class idEntity;

struct staticPState_s
{
	Vector2 origin;
	Vector2	axis;
	Vector2 localOrigin;
	Vector2	localAxis;
};

class idPhysics_Static : public idPhysics
{
public:
	CLASS_PROTOTYPE(idPhysics_Static);

	idPhysics_Static();
	
	~idPhysics_Static();
public:	// common physics interface
	void SetSelf(std::shared_ptr<idEntity> e) override;

	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const override;
	int GetNumClipModels() const override;

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

	void ClearContacts() override;
	void AddContactEntity(std::shared_ptr<idEntity> e) override;
	void RemoveContactEntity(std::shared_ptr<idEntity> e) override;
protected:
	std::shared_ptr<idEntity> self; // entity using this physics object
	staticPState_s current;			// physics state
	std::shared_ptr<idClipModel> clipModel; // collision model
};

#endif