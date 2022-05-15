#ifndef PHYSICS_STATIC_H
#define PHYSICS_STATIC_H

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
	idPhysics_Static(const idPhysics_Static&) = default;
	idPhysics_Static& operator=(const idPhysics_Static&) = default;
	idPhysics_Static(idPhysics_Static&&) = default;
	idPhysics_Static& operator=(idPhysics_Static&&) = default;

public:	// common physics interface
	void SetSelf(idEntity* e) noexcept override;
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const noexcept override;
	int GetNumClipModels() const noexcept override;

	void SetContents(int contents, int id = -1) override;
	int GetContents(int id = -1) const override;

	void SetClipMask(int mask, int id = -1) noexcept override;
	int GetClipMask(int id = -1) const noexcept override;

	const idBounds& GetBounds(int id = -1) const noexcept override;
	const idBounds& GetAbsBounds(int id = -1) const override;

	bool Evaluate(int timeStepMSec, int endTimeMSec) noexcept override;
	void UpdateTime(int endTimeMSec) noexcept override;
	int	GetTime() const noexcept override;

	void Activate() noexcept override;
	void PutToRest() noexcept override;
	bool IsAtRest() const noexcept override;

	void SaveState() noexcept override;
	void RestoreState() noexcept override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) noexcept override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) noexcept override;

	const Vector2 & GetOrigin(int id = 0) const noexcept override;
	const Vector2 &	GetAxis(int id = 0) const noexcept override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) noexcept override;
	const Vector2& GetLinearVelocity(int id = 0) const noexcept override;

	void DisableClip() noexcept override;
	void EnableClip() noexcept override;

	void UnlinkClip() noexcept override;
	void LinkClip() override;

	bool EvaluateContacts() noexcept override;

	void ClearContacts() noexcept override;
	void AddContactEntity(idEntity* e) noexcept override;
	void RemoveContactEntity(idEntity* e) noexcept override;
protected:
	idEntity* self; // entity using this physics object
	staticPState_s current;			// physics state
	std::shared_ptr<idClipModel> clipModel; // collision model
};

#endif