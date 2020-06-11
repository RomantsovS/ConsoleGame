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
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const override;
	int GetNumClipModels() const override;

	void SetClipMask(int mask, int id = -1) override;
	int GetClipMask(int id = -1) const override;

	const idBounds& GetBounds(int id = -1) const override;
	const idBounds& GetAbsBounds(int id = -1) const override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) override;
	void UpdateTime(int endTimeMSec) override;
	int	GetTime() const override;

	void Activate() override;
	void PutToRest() override;
	virtual bool IsAtRest() const override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) override;

	const Vector2 & GetOrigin(int id = 0) const override;
	const Vector2 &	GetAxis(int id = 0) const override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) override;
	virtual const Vector2& GetLinearVelocity(int id = 0) const override;

	void ClearContacts() override;
	virtual void AddContactEntity(std::shared_ptr<idEntity> e) override;
	virtual void RemoveContactEntity(std::shared_ptr<idEntity> e) override;
protected:
	std::weak_ptr<idEntity> self; // entity using this physics object
	int clipMask; // contents the physics object collides with
	std::vector<contactInfo_t> contacts; // contacts with other physics objects
	std::vector<contactEntity_t> contactEntities; // entities touching this physics object
protected:
	// add contact entity links to contact entities
	void AddContactEntitiesForContacts();
	// active all contact entities
	void ActivateContactEntities();
	// returns true if the whole physics object is outside the world bounds
	bool IsOutsideWorld() const;
};

#endif