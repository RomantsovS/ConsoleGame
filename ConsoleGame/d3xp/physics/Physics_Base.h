#ifndef PHYSICS_BASE_H
#define PHYSICS_BASE_H

#include <vector>

#include "Physics.h"
#include "../../cm/CollisionModel.h"

#define contactEntity_t std::shared_ptr<idEntity>

class idPhysics_Base : public idPhysics
{
public:
	CLASS_PROTOTYPE(idPhysics_Base);

	idPhysics_Base();

	~idPhysics_Base();

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
	virtual void AddContactEntity(std::shared_ptr<idEntity> e) override;
	virtual void RemoveContactEntity(std::shared_ptr<idEntity> e) override;
protected:
	std::shared_ptr<idEntity> self; // entity using this physics object
	int clipMask; // contents the physics object collides with
	std::vector<contactInfo_t> contacts; // contacts with other physics objects
	std::vector<contactEntity_t> contactEntities; // entities touching this physics object
protected:
	// add contact entity links to contact entities
	void AddContactEntitiesForContacts();
	// active all contact entities
	void ActivateContactEntities();
};

#endif