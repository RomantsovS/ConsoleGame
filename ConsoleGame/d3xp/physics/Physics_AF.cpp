#include <precompiled.h>
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_Base, idPhysics_AF)
END_CLASS

//===============================================================
//
//	idAFBody
//
//===============================================================

/*
================
idAFBody::idAFBody
================
*/
idAFBody::idAFBody() {
	Init();
}

/*
================
idAFBody::idAFBody
================
*/
idAFBody::idAFBody(const std::string& name, std::shared_ptr<idClipModel> clipModel, float density) {

	//assert(clipModel);
	//assert(clipModel->IsTraceModel());

	Init();

	this->name = name;
	this->clipModel = nullptr;

	SetClipModel(clipModel);

	current->worldOrigin = clipModel->GetOrigin();
	//current->worldAxis = clipModel->GetAxis();
	*next = *current;
}

/*
================
idAFBody::~idAFBody
================
*/
idAFBody::~idAFBody() {
	clipModel = nullptr;
}

/*
================
idAFBody::Init
================
*/
void idAFBody::Init() {
	name = "noname";
	parent = nullptr;
	clipModel = nullptr;

	clipMask = 0;

	current = &state[0];
	next = &state[1];
	current->worldOrigin = vec2_origin;
	current->spatialVelocity = vec2_origin;
	*next = *current;
	saved = *current;

	memset(&fl, 0, sizeof(fl));

	fl.selfCollision = true;
}

/*
================
idAFBody::SetClipModel
================
*/
void idAFBody::SetClipModel(std::shared_ptr<idClipModel> clipModel) {
	if (this->clipModel && this->clipModel != clipModel) {
		this->clipModel = nullptr;
	}
	this->clipModel = clipModel;
}

//===============================================================
//                                                        M
//  idPhysics_AF                                         MrE
//                                                        E
//===============================================================

/*
================
idPhysics_AF::Evolve
================
*/
void idPhysics_AF::Evolve(float timeStep) {
	// calculate the position of the bodies for the next physics state

	if (!bodies.empty()) {
		auto body = bodies[0];

		body->next->spatialVelocity = body->current->spatialVelocity;

		// translate world origin
		body->next->worldOrigin = body->current->worldOrigin + timeStep * body->next->spatialVelocity;
	}

	if (bodies.size() > 1) {
		for (size_t i = 1; i < bodies.size(); ++i) {
			auto body = bodies[i];

			body->next->spatialVelocity = body->current->spatialVelocity;

			// translate world origin
			body->next->worldOrigin = body->current->worldOrigin + timeStep * body->next->spatialVelocity;
		}
	}
}

/*
================
idPhysics_AF::CollisionImpulse

  apply impulse to the colliding bodies
  the current state of the body should be set to the moment of impact
  this is silly as it doesn't take the AF structure into account
================
*/
bool idPhysics_AF::CollisionImpulse(float timeStep, idAFBody* body, trace_t& collision) {
	auto ent = gameLocal.entities[collision.c.entityNum];
	if (ent == self.lock()) {
		return false;
	}

	auto velocity = body->current->spatialVelocity;

	// callback to self to let the entity know about the impact
	return self.lock()->Collide(collision, velocity);
}

/*
================
idPhysics_AF::ApplyCollisions
================
*/
bool idPhysics_AF::ApplyCollisions(float timeStep) {
	for (size_t i = 0; i < collisions.size(); i++) {
		if (CollisionImpulse(timeStep, collisions[i].body.get(), collisions[i].trace)) {
			return true;
		}
	}
	return false;
}

/*
================
idPhysics_AF::SetupCollisionForBody
================
*/
idEntity* idPhysics_AF::SetupCollisionForBody(idAFBody* body) const {
	size_t i;
	idEntity* passEntity = nullptr;

	if (!selfCollision || !body->fl.selfCollision) {

		// disable all bodies
		for (i = 0; i < bodies.size(); i++) {
			bodies[i]->clipModel->Disable();
		}

	}
	else {

		// enable all bodies that have self collision
		for (i = 0; i < bodies.size(); i++) {
			if (bodies[i]->fl.selfCollision) {
				bodies[i]->clipModel->Enable();
			}
			else {
				bodies[i]->clipModel->Disable();
			}
		}

		// don't let the body collide with itself
		body->clipModel->Disable();
	}

	return passEntity;
}

/*
================
idPhysics_AF::CheckForCollisions

  check for collisions between the current and next state
  if there is a collision the next state is set to the state at the moment of impact
  assumes all bodies are linked for collision detection and relinks all bodies after moving them
================
*/
void idPhysics_AF::CheckForCollisions(float timeStep) {
	trace_t collision;

	// clear list with collisions
	collisions.clear();

	if (!enableCollision) {
		return;
	}

	for (size_t i = 0; i < bodies.size(); i++) {
		auto body = bodies[i];

		if (body->clipMask != 0) {

			auto passEntity = SetupCollisionForBody(body.get());

			// if there was a collision
			if (gameLocal.clip.Motion(collision, body->current->worldOrigin, body->next->worldOrigin,
				body->clipModel.get(), body->clipMask, passEntity)) {

				// set the next state to the state at the moment of impact
				body->next->worldOrigin = collision.endpos;

				// add collision to the list
				auto index = collisions.size();
				collisions.resize(index + 1);
				collisions[index].trace = collision;
				collisions[index].body = body;
			}
		}

		body->clipModel->Link(gameLocal.clip, self.lock(), body->clipModel->GetId(), body->next->worldOrigin);
	}

	MoveEachBodiesToPrevOne();
}

void idPhysics_AF::MoveEachBodiesToPrevOne() {
	auto body = bodies[0];

	if (bodies.size() < 2)
		return;

	auto head_body_moved = body->current->worldOrigin.GetIntegerVectorFloor() != body->next->worldOrigin.GetIntegerVectorFloor();

	// translate world origin
	for (size_t i = 1; i < bodies.size(); ++i) {
		if (head_body_moved) {
			bodies[i]->next->worldOrigin = bodies[i - 1]->current->worldOrigin;
		}
		else {
			bodies[i]->next->worldOrigin = bodies[i]->current->worldOrigin;
		}
		
		bodies[i]->clipModel->Link(gameLocal.clip, self.lock(), bodies[i]->clipModel->GetId(), bodies[i]->next->worldOrigin);
	}
}

/*
================
idPhysics_AF::EvaluateContacts
================
*/
bool idPhysics_AF::EvaluateContacts() {
	/*int i, j, k, numContacts, numBodyContacts;
	idAFBody* body;
	contactInfo_t contactInfo[10];
	idEntity* passEntity;
	idVecX dir(6, VECX_ALLOCA(6));

	// evaluate bodies
	EvaluateBodies(current.lastTimeStep);*/

	// remove all existing contacts
	ClearContacts();

	/*contactBodies.SetNum(0);

	if (!enableCollision) {
		return false;
	}

	// find all the contacts
	for (i = 0; i < bodies.Num(); i++) {
		body = bodies[i];

		if (body->clipMask == 0) {
			continue;
		}

		passEntity = SetupCollisionForBody(body);

		body->InverseWorldSpatialInertiaMultiply(dir, body->current->externalForce.ToFloatPtr());
		dir.SubVec6(0) = body->current->spatialVelocity + current.lastTimeStep * dir.SubVec6(0);
		dir.SubVec3(0).Normalize();
		dir.SubVec3(1).Normalize();

		numContacts = gameLocal.clip.Contacts(contactInfo, 10, body->current->worldOrigin, dir.SubVec6(0), 2.0f, //CONTACT_EPSILON,
			body->clipModel, body->current->worldAxis, body->clipMask, passEntity);

		// merge nearby contacts between the same bodies
		// and assure there are at most three planar contacts between any pair of bodies
		for (j = 0; j < numContacts; j++) {

			numBodyContacts = 0;
			for (k = 0; k < contacts.size(); k++) {
				if (contacts[k].entityNum == contactInfo[j].entityNum) {
					if ((contacts[k].id == i && contactInfo[j].id == contactBodies[k]) ||
						(contactBodies[k] == i && contacts[k].id == contactInfo[j].id)) {

						if ((contacts[k].point - contactInfo[j].point).LengthSqr() < Square(2.0f)) {
							break;
						}
						if (idMath::Fabs(contacts[k].normal * contactInfo[j].normal) > 0.9f) {
							numBodyContacts++;
						}
					}
				}
			}

			if (k >= contacts.size() && numBodyContacts < 3) {
				contacts.push_back(contactInfo[j]);
				contactBodies.Append(i);
			}
		}

	}

	AddContactEntitiesForContacts();*/

	return (contacts.size() != 0);
}

/*
================
idPhysics_AF::SwapStates
================
*/
void idPhysics_AF::SwapStates() {
	for (size_t i = 0; i < bodies.size(); i++) {

		auto body = bodies[i];

		// swap the current and next state for next simulation step
		auto swap = body->current;
		body->current = body->next;
		body->next = swap;
	}
}

/*
================
idPhysics_AF::UpdateClipModels
================
*/
void idPhysics_AF::UpdateClipModels() {
	for (size_t i = 0; i < bodies.size(); i++) {
		auto body = bodies[i];
		body->clipModel->Link(gameLocal.clip, self.lock(), body->clipModel->GetId(), body->current->worldOrigin);
	}
}

/*
================
idPhysics_AF::TestIfAtRest
================
*/
bool idPhysics_AF::TestIfAtRest(float timeStep) {
	if (current.atRest >= 0) {
		return true;
	}

	current.activateTime += timeStep;

	return false;
}

/*
================
idPhysics_AF::Rest
================
*/
void idPhysics_AF::Rest() {
	current.atRest = gameLocal.time;

	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->current->spatialVelocity.Zero();
	}

	self.lock()->BecomeInactive(TH_PHYSICS);
}

/*
================
idPhysics_AF::Activate
================
*/
void idPhysics_AF::Activate() {
	// if the articulated figure was at rest
	if (current.atRest >= 0) {
		// reset the active time for the max move time
		current.activateTime = 0.0f;
	}
	current.atRest = -1;
	current.noMoveTime = 0.0f;
	self.lock()->BecomeActive(TH_PHYSICS);
}

/*
================
idPhysics_AF::PutToRest

  put to rest untill something collides with this physics object
================
*/
void idPhysics_AF::PutToRest() {
	Rest();
}

/*
================
idPhysics_AF::SetClipModel
================
*/
void idPhysics_AF::SetClipModel(std::shared_ptr<idClipModel> model, float density, int id, bool freeOld) {
}

/*
================
idPhysics_AF::GetClipModel
================
*/
std::shared_ptr<idClipModel> idPhysics_AF::GetClipModel(int id) const {
	if (id >= 0 && id < static_cast<int>(bodies.size())) {
		return bodies[id]->GetClipModel();
	}
	return nullptr;
}

/*
================
idPhysics_AF::GetNumClipModels
================
*/
int idPhysics_AF::GetNumClipModels() const {
	return bodies.size();
}

/*
================
idPhysics_AF::GetBounds
================
*/
const idBounds& idPhysics_AF::GetBounds(int id) const {
	static idBounds relBounds;

	if (id >= 0 && id < static_cast<int>(bodies.size())) {
		return bodies[id]->GetClipModel()->GetBounds();
	}
	else if (!bodies.size()) {
		relBounds.Zero();
		return relBounds;
	}
	else {
		/*relBounds = bodies[0]->GetClipModel()->GetBounds();
		for (i = 1; i < bodies.size(); i++) {
			idBounds bounds;
			Vector2 origin = (bodies[i]->GetWorldOrigin() - bodies[0]->GetWorldOrigin()) * bodies[0]->GetWorldAxis().Transpose();
			idMat3 axis = bodies[i]->GetWorldAxis() * bodies[0]->GetWorldAxis().Transpose();
			bounds.FromTransformedBounds(bodies[i]->GetClipModel()->GetBounds(), origin, axis);
			relBounds += bounds;
		}
		return relBounds;*/
		gameLocal.Warning("Error Physics_AF GetBounds()");
	}

	return idBounds::GetBoundsZero();
}

/*
================
idPhysics_AF::GetAbsBounds
================
*/
const idBounds& idPhysics_AF::GetAbsBounds(int id) const {
	static idBounds absBounds;

	if (id >= 0 && id < static_cast<int>(bodies.size())) {
		return bodies[id]->GetClipModel()->GetAbsBounds();
	}
	else if (!bodies.size()) {
		absBounds.Zero();
		return absBounds;
	}
	else {
		absBounds = bodies[0]->GetClipModel()->GetAbsBounds();
		for (size_t i = 1; i < static_cast<int>(bodies.size()); i++) {
			absBounds += bodies[i]->GetClipModel()->GetAbsBounds();
		}
		return absBounds;
	}
}

/*
================
idPhysics_AF::Evaluate
================
*/
bool idPhysics_AF::Evaluate(int timeStepMSec, int endTimeMSec) {
	float timeStep;

	timeStep = MS2SEC(timeStepMSec);
	current.lastTimeStep = timeStep;

	// if the simulation is suspended because the figure is at rest
	if (current.atRest >= 0 || timeStep <= 0.0f) {
		DebugDraw();
		return false;
	}

	// evaluate contacts
	EvaluateContacts();

	// evolve current state to next state
	Evolve(timeStep);

	// debug graphics
	DebugDraw();

	// check for collisions between current and next state
	CheckForCollisions(timeStep);

#ifdef AF_TIMINGS
	timer_collision.Stop();
#endif

	// swap the current and next state
	SwapStates();

	// make sure all clip models are disabled in case they were enabled for self collision
	/*if (selfCollision) {
		DisableClip();
	}*/
	EnableClip();

	// apply collision impulses
	if (ApplyCollisions(timeStep)) {
		current.atRest = gameLocal.time;
		comeToRest = true;
	}

	// test if the simulation can be suspended because the whole figure is at rest
	if (comeToRest && TestIfAtRest(timeStep)) {
		Rest();
	}
	else {
		ActivateContactEntities();
	}

	if (IsOutsideWorld()) {
		gameLocal.Warning("articulated figure moved outside world bounds for entity '%s' type '%s' at (%s)",
			self.lock()->name.c_str(), self.lock()->GetType()->classname.c_str(),
			bodies[0]->current->worldOrigin.ToString(0).c_str());
		Rest();
	}
	
	return true;
}

/*
================
idPhysics_AF::UpdateTime
================
*/
void idPhysics_AF::UpdateTime(int endTimeMSec) {
}

/*
================
idPhysics_AF::GetTime
================
*/
int idPhysics_AF::GetTime() const {
	return gameLocal.time;
}

/*
================
idPhysics_AF::DebugDraw
================
*/
void idPhysics_AF::DebugDraw() {
}

/*
================
idPhysics_AF::idPhysics_AF
================
*/
idPhysics_AF::idPhysics_AF() {
	bodies.clear();
	contacts.clear();
	collisions.clear();

	memset(&current, 0, sizeof(current));
	current.atRest = -1;
	current.lastTimeStep = 0.0f;
	saved = current;

	enableCollision = true;
	selfCollision = true;
	comeToRest = true;
}

/*
================
idPhysics_AF::~idPhysics_AF
================
*/
idPhysics_AF::~idPhysics_AF() {
	size_t i;

	for (i = 0; i < bodies.size(); i++) {
		bodies[i] = nullptr;
	}
}

/*
================
idPhysics_AF::AddBody

  bodies get an id in the order they are added starting at zero
  as such the first body added will get id zero
================
*/
int idPhysics_AF::AddBody(const std::shared_ptr<idAFBody>& body) {
	int id = 0;

	if (body->clipModel == nullptr) {
		gameLocal.Error("idPhysics_AF::AddBody: body '%s' has no clip model.", body->name.c_str());
		return 0;
	}

	if (std::find(bodies.begin(), bodies.end(), body) != bodies.end()) {
		gameLocal.Error("idPhysics_AF::AddBody: body '%s' added twice.", body->name.c_str());
	}

	if (GetBody(body->name)) {
		gameLocal.Error("idPhysics_AF::AddBody: a body with the name '%s' already exists.", body->name.c_str());
	}

	id = bodies.size();
	//body->clipModel->SetId(id);
	/*if (body->linearFriction < 0.0f) {
		body->linearFriction = linearFriction;
		body->angularFriction = angularFriction;
		body->contactFriction = contactFriction;
	}
	if (body->bouncyness < 0.0f) {
		body->bouncyness = bouncyness;
	}*/
	if (!body->fl.clipMaskSet) {
		body->clipMask = clipMask;
	}

	bodies.push_back(body);

	return id;
}

/*
================
idPhysics_AF::GetBodyId
================
*/
int idPhysics_AF::GetBodyId(const std::shared_ptr<idAFBody>& body) const {
	auto iter = std::find(bodies.begin(), bodies.end(), body);

	if (iter == bodies.end() && body) {
		gameLocal.Error("GetBodyId: body '%s' is not part of the articulated figure.\n", body->name.c_str());
	}
	return iter - bodies.begin();
}

/*
================
idPhysics_AF::GetBody
================
*/
std::shared_ptr<idAFBody> idPhysics_AF::GetBody(const std::string& bodyName) const {
	for (size_t i = 0; i < bodies.size(); i++) {
		if (bodies[i]->name == bodyName) {
			return bodies[i];
		}
	}

	return nullptr;
}

/*
================
idPhysics_AF::DeleteBody
================
*/
void idPhysics_AF::DeleteBody(const int id) {
	if (id < 0 || id > static_cast<int>(bodies.size())) {
		gameLocal.Error("DeleteBody: no body with id %d.", id);
		return;
	}

	// remove the body
	bodies[id]->SetClipModel(nullptr);
	bodies[id] = nullptr;
	bodies.erase(bodies.begin() + id);

	// set new body ids
	for (size_t j = 0; j < bodies.size(); j++) {
		bodies[j]->clipModel->SetId(j);
	}
}

/*
================
idPhysics_AF::IsAtRest
================
*/
bool idPhysics_AF::IsAtRest() const {
	return current.atRest >= 0;
}

/*
================
idPhysics_AF::SaveState
================
*/
void idPhysics_AF::SaveState() {
	saved = current;

	for (size_t i = 0; i < bodies.size(); i++) {
		memcpy(&bodies[i]->saved, bodies[i]->current, sizeof(AFBodyPState_t));
	}
}

/*
================
idPhysics_AF::RestoreState
================
*/
void idPhysics_AF::RestoreState() {
	current = saved;

	for (size_t i = 0; i < bodies.size(); i++) {
		*(bodies[i]->current) = bodies[i]->saved;
	}

	EvaluateContacts();
}

/*
================
idPhysics_AF::SetOrigin
================
*/
void idPhysics_AF::SetOrigin(const Vector2& newOrigin, int id) {
	/*if (masterBody) {
		Translate(masterBody->current->worldOrigin + masterBody->current->worldAxis * newOrigin - bodies[0]->current->worldOrigin);
	}
	else {*/
		Translate(newOrigin - bodies[0]->current->worldOrigin);
	//}
}

/*
================
idPhysics_AF::SetAxis
================
*/
void idPhysics_AF::SetAxis(const Vector2& newAxis, int id) {
}

/*
================
idPhysics_AF::Translate
================
*/
void idPhysics_AF::Translate(const Vector2& translation, int id) {
	// translate all the bodies
	for (size_t i = 0; i < bodies.size(); i++) {

		auto body = bodies[i];
		body->current->worldOrigin += translation;
	}

	Activate();

	UpdateClipModels();
}

/*
================
idPhysics_AF::GetOrigin
================
*/
const Vector2& idPhysics_AF::GetOrigin(int id) const {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return vec2_origin;
	}
	else {
		return bodies[id]->current->worldOrigin;
	}
}

/*
================
idPhysics_AF::SetLinearVelocity
================
*/
void idPhysics_AF::SetLinearVelocity(const Vector2& newLinearVelocity, int id) {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return;
	}
	bodies[id]->current->spatialVelocity = newLinearVelocity;
	Activate();
}

/*
================
idPhysics_AF::GetLinearVelocity
================
*/
const Vector2& idPhysics_AF::GetLinearVelocity(int id) const {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return vec2_origin;
	}
	else {
		return bodies[id]->current->spatialVelocity;
	}
}

/*
================
idPhysics_AF::DisableClip
================
*/
void idPhysics_AF::DisableClip() {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Disable();
	}
}

/*
================
idPhysics_AF::EnableClip
================
*/
void idPhysics_AF::EnableClip() {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Enable();
	}
}

/*
================
idPhysics_AF::UnlinkClip
================
*/
void idPhysics_AF::UnlinkClip() {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Unlink();
	}
}

/*
================
idPhysics_AF::LinkClip
================
*/
void idPhysics_AF::LinkClip() {
	UpdateClipModels();
}