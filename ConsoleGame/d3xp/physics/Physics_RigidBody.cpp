#include "Physics_RigidBody.h"
#include "../Game_local.h"
#include "../../idlib/math/Math.h"

CLASS_DECLARATION(idPhysics_Base, idPhysics_RigidBody)

idPhysics_RigidBody::idPhysics_RigidBody() {
	// set default rigid body properties
	//SetClipMask(MASK_SOLID);
	//SetBouncyness(0.6f);
	//SetFriction(0.6f, 0.6f, 0.0f);
	clipModel = nullptr;

	current.atRest = -1;
	current.lastTimeStep = 0.0f;

	current.i.position.Zero();
	//current.i.orientation.Identity();

	//current.i.linearMomentum.Zero();
	//current.i.angularMomentum.Zero();

	saved = current;

	/*mass = 1.0f;
	inverseMass = 1.0f;
	centerOfMass.Zero();
	inertiaTensor.Identity();
	inverseInertiaTensor.Identity();

	// use the least expensive euler integrator
	integrator = new (TAG_PHYSICS) idODE_Euler(sizeof(rigidBodyIState_t) / sizeof(float), RigidBodyDerivatives, this);

	dropToFloor = false;
	noImpact = false;
	noContact = false;

	hasMaster = false;
	isOrientated = false;*/
}

idPhysics_RigidBody::~idPhysics_RigidBody() {
	if (clipModel) {
		clipModel = nullptr;
	}
	//delete integrator;
}

/*
================
idPhysics_RigidBody::GetClipModel
================
*/
std::shared_ptr<idClipModel> idPhysics_RigidBody::GetClipModel(int id) const {
	return clipModel;
}

/*
================
idPhysics_RigidBody::GetNumClipModels
================
*/
int idPhysics_RigidBody::GetNumClipModels() const
{
	return 1;
}

/*
================
idPhysics_RigidBody::Evaluate

  Evaluate the impulse based rigid body physics.
  When a collision occurs an impulse is applied at the moment of impact but
  the remaining time after the collision is ignored.
================
*/
bool idPhysics_RigidBody::Evaluate(int timeStepMSec, int endTimeMSec)
{
	rigidBodyPState_t next_step;
	trace_t collision;
	Vector2 impulse;
	float timeStep;
	bool collided;

	timeStep = MS2SEC(timeStepMSec);
	current.lastTimeStep = timeStep;

	// if the body is at rest
	if (current.atRest >= 0 || timeStep <= 0.0f) {
		DebugDraw();
		return false;
	}

	clipModel->Unlink();

	next_step = current;

	// calculate next position and orientation
	Integrate(timeStep, next_step);

	// check for collisions from the current to the next state
	collided = CheckForCollisions(timeStep, next_step, collision);

	// set the new state
	current = next_step;

	if (collided) {
		// apply collision impulse
		if (CollisionImpulse(collision, impulse)) {
			current.atRest = gameLocal.time;
		}
	}

	// update the position of the clip model
	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position);

	DebugDraw();


	if (!noContact) {
		// get contacts
		EvaluateContacts();
	}

	if (current.atRest < 0) {
		ActivateContactEntities();
	}

	return true;
}

void idPhysics_RigidBody::UpdateTime(int endTimeMSec) {
}

int idPhysics_RigidBody::GetTime() const {
	return gameLocal.time;
}

void idPhysics_RigidBody::Activate()
{
	current.atRest = -1;
	self->BecomeActive(TH_PHYSICS);
}

/*
================
idPhysics_RigidBody::SaveState
================
*/
void idPhysics_RigidBody::SaveState() {
	saved = current;
}

/*
================
idPhysics_RigidBody::RestoreState
================
*/
void idPhysics_RigidBody::RestoreState() {
	current = saved;

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position);

	EvaluateContacts();
}

bool idPhysics_RigidBody::EvaluateContacts()
{
	Vector2 dir;

	ClearContacts();

	contacts.resize(10);

	/*dir.SubVec3(0) = current.i.linearMomentum + current.lastTimeStep * gravityVector * mass;
	dir.SubVec3(1) = current.i.angularMomentum;
	dir.SubVec3(0).Normalize();
	dir.SubVec3(1).Normalize();*/
	dir = vec2_origin;
	auto num = gameLocal.clip.Contacts(contacts, 10, clipModel->GetOrigin(),
		dir, CONTACT_EPSILON, clipModel, /*clipModel->GetAxis(),*/ clipMask, self);
	contacts.reserve(num);

	AddContactEntitiesForContacts();

	return !contacts.empty();
}

void idPhysics_RigidBody::Integrate(const float deltaTime, rigidBodyPState_t& next_)
{
	Vector2 position;

	position = current.i.position;
	//current.i.position += centerOfMass * current.i.orientation;

	//current.i.orientation.TransposeSelf();

	//integrator->Evaluate((float*)&current.i, (float*)&next_.i, 0, deltaTime);
	//next_.i.orientation.OrthoNormalizeSelf();

	// apply gravity
	//next_.i.linearMomentum += deltaTime * gravityVector * mass;

	//current.i.orientation.TransposeSelf();
	//next_.i.orientation.TransposeSelf();

	current.i.position = position;
	//next_.i.position -= centerOfMass * next_.i.orientation;

	next_.atRest = current.atRest;
}

/*
================
idPhysics_RigidBody::CheckForCollisions

  Check for collisions between the current and next state.
  If there is a collision the next state is set to the state at the moment of impact.
================
*/
bool idPhysics_RigidBody::CheckForCollisions(const float deltaTime, rigidBodyPState_t& next_, trace_t& collision)
{
	bool collided = false;

	// if there was a collision
	if (gameLocal.clip.Motion(collision, current.i.position, next_.i.position, clipModel, clipMask, self)) {
		// set the next state to the state at the moment of impact
		next_.i.position = collision.endpos;
		//next_.i.orientation = collision.endAxis;
		//next_.i.linearMomentum = current.i.linearMomentum;
		//next_.i.angularMomentum = current.i.angularMomentum;
		collided = true;
	}

	return collided;
}

bool idPhysics_RigidBody::CollisionImpulse(const trace_t& collision, Vector2& impulse)
{
	Vector2 velocity;

	// callback to self to let the entity know about the collision
	return self->Collide(collision, velocity);
}

void idPhysics_RigidBody::DebugDraw()
{
}

/*
================
idPhysics::SetOrigin
================
*/
void idPhysics_RigidBody::SetOrigin(const Vector2 &newOrigin, int id) {
	current.localOrigin = newOrigin;
	/*if (hasMaster) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.i.position = masterOrigin + newOrigin * masterAxis;
	}
	else {*/
		current.i.position = newOrigin;
	//}

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position);

	Activate();
}

/*
================
idPhysics::SetAxis
================
*/
void idPhysics_RigidBody::SetAxis(const Vector2& newAxis, int id) {
	/*idVec3 masterOrigin;
	idMat3 masterAxis;

	current.localAxis = newAxis;
	if (hasMaster && isOrientated) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.i.orientation = newAxis * masterAxis;
	}
	else {
		current.i.orientation = newAxis;
	}

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), clipModel->GetOrigin(), current.i.orientation);

	Activate();*/
}

void idPhysics_RigidBody::Translate(const Vector2& translation, int id)
{
	current.localOrigin += translation;
	current.i.position += translation;

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position);

	Activate();
}

void idPhysics_RigidBody::Rotate(const Vector2& rotation, int id)
{
	/*idVec3 masterOrigin;
	idMat3 masterAxis;*/

	//current.i.orientation *= rotation.ToMat3();
	current.i.position *= rotation;

	/*if (hasMaster) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.localAxis *= rotation.ToMat3();
		current.localOrigin = (current.i.position - masterOrigin) * masterAxis.Transpose();
	}
	else {*/
		//current.localAxis = current.i.orientation;
		current.localOrigin = current.i.position;
	//}

	clipModel->Link(gameLocal.clip, self, clipModel->GetId(), current.i.position);

	Activate();
}

/*
================
idPhysics_RigidBody::GetOrigin
================
*/
const Vector2& idPhysics_RigidBody::GetOrigin(int id) const {
	return current.i.position;
}
