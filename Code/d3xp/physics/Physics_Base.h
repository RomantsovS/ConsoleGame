#ifndef PHYSICS_BASE_H
#define PHYSICS_BASE_H

#define contactEntity_t idEntity*

class idPhysics_Base : public idPhysics
{
public:
	CLASS_PROTOTYPE(idPhysics_Base);

	idPhysics_Base();
	~idPhysics_Base();
	idPhysics_Base(const idPhysics_Base&) = default;
	idPhysics_Base& operator=(const idPhysics_Base&) = default;
	idPhysics_Base(idPhysics_Base&&) = default;
	idPhysics_Base& operator=(idPhysics_Base&&) = default;

public:	// common physics interface

	void SetSelf(idEntity* e) noexcept override;
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) noexcept override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const noexcept override;
	int GetNumClipModels() const noexcept override;

	void SetClipMask(int mask, int id = -1) noexcept override;
	int GetClipMask(int id = -1) const noexcept override;

	const idBounds& GetBounds(int id = -1) const noexcept override;
	const idBounds& GetAbsBounds(int id = -1) const noexcept override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) noexcept override;
	void UpdateTime(int endTimeMSec) noexcept override;
	int GetTime() const noexcept override;

	void Activate() noexcept override;
	void PutToRest() noexcept override;
	bool IsAtRest() const noexcept override;

	void SaveState() noexcept override;
	void RestoreState() noexcept override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) noexcept override;
	void SetAxis(const Vector2 &newAxis, int id = -1) noexcept override;

	void Translate(const Vector2 &translation, int id = -1) noexcept override;
	void Rotate(const Vector2 &rotation, int id = -1) noexcept override;

	const Vector2 & GetOrigin(int id = 0) const noexcept override;
	const Vector2 &	GetAxis(int id = 0) const noexcept override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) noexcept override;
	const Vector2& GetLinearVelocity(int id = 0) const noexcept override;

	void DisableClip() noexcept override;
	void EnableClip() noexcept override;

	void UnlinkClip() noexcept override;
	void LinkClip() noexcept override;

	bool EvaluateContacts() noexcept override;
	void ClearContacts() override;
	void AddContactEntity(idEntity* e) override;
	void RemoveContactEntity(idEntity* e) noexcept override;
protected:
	idEntity* self; // entity using this physics object
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