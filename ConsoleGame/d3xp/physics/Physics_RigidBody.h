#ifndef PHYSICS_RIDIGBODY_H
#define PHYSICS_RIDIGBODY_H

#include <memory>

#include "Physics_Base.h"
#include "Clip.h"
#include "../../cm/CollisionModel.h"
#include "../../idlib/math/Ode.h"

struct rigidBodyIState_t {
	Vector2 position; // position of trace model
	//idMat3 orientation; // orientation of trace model
	Vector2 linearMomentum; // translational momentum relative to center of mass
	//idVec3 angularMomentum; // rotational momentum relative to center of mass

	rigidBodyIState_t() :
		position(vec2_origin),
		//orientation(mat3_identity),
		linearMomentum(vec2_origin)
		//angularMomentum(vec3_zero) 
	{
	}
};

struct rigidBodyPState_t {
	int atRest; // set when simulation is suspended
	float lastTimeStep; // length of last time step
	Vector2 localOrigin; // origin relative to master
	//idMat3 localAxis; // axis relative to master
	//idVec6 pushVelocity; // push velocity
	//Vector2 externalForce; // external force relative to center of mass
	//Vector2 externalTorque; // external torque relative to center of mass
	rigidBodyIState_t i; // state used for integration

	rigidBodyPState_t() :
		atRest(true),
		lastTimeStep(0),
		localOrigin(vec2_origin)
		//localAxis(mat3_identity),
		//pushVelocity(vec6_zero),
		//externalForce(vec2_origin),
		//externalTorque(vec2_origin) 
	{
	}
};

class idPhysics_RigidBody : public idPhysics_Base
{
public:
	CLASS_PROTOTYPE(idPhysics_RigidBody);

	idPhysics_RigidBody();
	~idPhysics_RigidBody();

public:	// common physics interface
	void SetClipModel(std::shared_ptr<idClipModel> model, float density, int id = 0, bool freeOld = true) override;
	std::shared_ptr<idClipModel> GetClipModel(int id = 0) const;
	int GetNumClipModels() const;

	const idBounds& GetBounds(int id = -1) const;
	const idBounds& GetAbsBounds(int id = -1) const;

	bool Evaluate(int timeStepMSec, int endTimeMSec) override;
	void UpdateTime(int endTimeMSec) override;
	int GetTime() const override;

	void Activate() override;
	void PutToRest() override;
	bool IsAtRest() const override;

	void SaveState() override;
	void RestoreState() override;

	void SetOrigin(const Vector2 &newOrigin, int id = -1) override;
	void SetAxis(const Vector2 &newAxis, int id = -1) override;

	void Translate(const Vector2 &translation, int id = -1) override;
	void Rotate(const Vector2 &rotation, int id = -1) override;

	const Vector2 & GetOrigin(int id = 0) const override;
	//const Vector2 &	GetAxis(int id = 0) const override;

	void SetLinearVelocity(const Vector2& newLinearVelocity, int id = 0) override;
	virtual const Vector2& GetLinearVelocity(int id = 0) const override;

	bool EvaluateContacts();
private:
	// state of the rigid body
	rigidBodyPState_t current;
	rigidBodyPState_t saved;

	std::shared_ptr<idClipModel> clipModel; // clip model used for collision detection

	std::shared_ptr<idODE> integrator; // integrator
	bool noContact; // if true do not determine contacts and no contact friction

private:
	friend void RigidBodyDerivatives(const float t, const void* clientData, const float* state, std::vector<float>& derivatives);
	void Integrate(const float deltaTime, rigidBodyPState_t& next_);
	bool CheckForCollisions(const float deltaTime, rigidBodyPState_t& next, trace_t& collision);
	bool CollisionImpulse(const trace_t& collision, Vector2& impulse);
	bool TestIfAtRest() const;
	void Rest();
	void DebugDraw();
};

#endif