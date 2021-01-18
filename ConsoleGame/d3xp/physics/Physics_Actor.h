#ifndef PHYSICS_PHYSICS_ACTOR_H_
#define PHYSICS_PHYSICS_ACTOR_H_

/*
===================================================================================

	Actor physics base class

	An actor typically uses one collision model which is aligned with the gravity
	direction. The collision model is usually a simple box with the origin at the
	bottom center.

===================================================================================
*/

class idPhysics_Actor : public idPhysics_Base {

public:
	CLASS_PROTOTYPE(idPhysics_Actor);

	idPhysics_Actor();
	~idPhysics_Actor();
	
	// align the clip model with the gravity direction
	void SetClipModelAxis();
public:	// common physics interface
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const override;
	int GetNumClipModels() const override;

	const idBounds& GetBounds(int id = -1) const override;
	const idBounds& GetAbsBounds(int id = -1) const override;

	const Vector2& GetOrigin(int id = 0) const override;

	void DisableClip() override;
	void EnableClip() override;

	void UnlinkClip() override;
	void LinkClip() override;

	bool EvaluateContacts() override;

protected:
	std::shared_ptr<idClipModel> clipModel;			// clip model used for collision detection
};

#endif // !PHYSICS_PHYSICS_ACTOR_H_
