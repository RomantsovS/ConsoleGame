#pragma hdrstop
#include "idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_PlayerBase, Physics_PlayerChain)
END_CLASS

/*
==================
Physics_PlayerChain::Friction

Handles both ground friction and water friction
==================
*/
void Physics_PlayerChain::Friction() noexcept {
}

/*
===================
Physics_PlayerChain::WalkMove
===================
*/
void Physics_PlayerChain::WalkMove() noexcept {
	Physics_PlayerChain::Friction();

	if (GetUserCmd().forwardmove > 0) {
		SetLinearVelocity(Vector2(0.0f, GetPlayerSpeed()));
	}
	else if (GetUserCmd().forwardmove < 0) {
		SetLinearVelocity(Vector2(0.0f, -GetPlayerSpeed()));
	}
	else if (GetUserCmd().rightmove > 0) {
		SetLinearVelocity(Vector2(GetPlayerSpeed(), 0.0f));
	}
	else if (GetUserCmd().rightmove < 0) {
		SetLinearVelocity(Vector2(-GetPlayerSpeed(), 0.0f));
	}
	else {
		auto linearvel = GetLinearVelocity();

		if (linearvel.x > 0)
			SetLinearVelocity(Vector2(GetPlayerSpeed(), 0.0f));
		else if (linearvel.x < 0)
			SetLinearVelocity(Vector2(-GetPlayerSpeed(), 0.0f));
		else if (linearvel.y > 0)
			SetLinearVelocity(Vector2(0.0f, GetPlayerSpeed()));
		else if (linearvel.y < 0)
			SetLinearVelocity(Vector2(0.0f, -GetPlayerSpeed()));
	}

	// evolve current state to next state
	Evolve(GetFrameTime());
}

/*
================
Physics_PlayerChain::MovePlayer
================
*/
void Physics_PlayerChain::MovePlayer(int msec) noexcept {

	idPhysics_PlayerBase::MovePlayer(msec);

	// move the player velocity into the frame of a pusher
	//current.velocity -= current.pushVelocity;

	// walking on ground
	Physics_PlayerChain::WalkMove();

	// move the player velocity back into the world frame
	//current.velocity += current.pushVelocity;
	//current.pushVelocity.Zero();
}

/*
================
Physics_PlayerChain::Physics_PlayerChain
================
*/
Physics_PlayerChain::Physics_PlayerChain() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "Physics_PlayerChain");
#endif // DEBUG_PRINT_Ctor_Dtor
	clipModel = nullptr;
	clipMask = 0;
	memset(&current, 0, sizeof(current));
	saved = current;

	bodies.clear();
	contacts.clear();
	collisions.clear();
}

/*
================
Physics_PlayerChain::~Physics_PlayerChain
================
*/
Physics_PlayerChain::~Physics_PlayerChain() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "Physics_PlayerChain");
#endif // DEBUG_PRINT_Ctor_Dtor
	size_t i;

	for (i = 0; i < bodies.size(); i++) {
		bodies[i] = nullptr;
	}
}

/*
================
Physics_PlayerChain::AddBody

  bodies get an id in the order they are added starting at zero
  as such the first body added will get id zero
================
*/
int Physics_PlayerChain::AddBody(const std::shared_ptr<idAFBody>& body) {
	int id = 0;

	if (body->clipModel == nullptr) {
		gameLocal.Error("Physics_PlayerChain::AddBody: body '%s' has no clip model.", body->name.c_str());
		return 0;
	}

	if (std::find(bodies.begin(), bodies.end(), body) != bodies.end()) {
		gameLocal.Error("Physics_PlayerChain::AddBody: body '%s' added twice.", body->name.c_str());
	}

	if (GetBody(body->name)) {
		gameLocal.Error("Physics_PlayerChain::AddBody: a body with the name '%s' already exists.", body->name.c_str());
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
Physics_PlayerChain::GetBodyId
================
*/
int Physics_PlayerChain::GetBodyId(const std::shared_ptr<idAFBody>& body) const {
	auto iter = std::find(bodies.begin(), bodies.end(), body);

	if (iter == bodies.end() && body) {
		gameLocal.Error("GetBodyId: body '%s' is not part of the articulated figure.\n", body->name.c_str());
	}
	return iter - bodies.begin();
}

/*
================
Physics_PlayerChain::GetBody
================
*/
std::shared_ptr<idAFBody> Physics_PlayerChain::GetBody(const std::string& bodyName) const noexcept {
	for (size_t i = 0; i < bodies.size(); i++) {
		if (bodies[i]->name == bodyName) {
			return bodies[i];
		}
	}

	return nullptr;
}

/*
================
idPhysics_AF::GetBody
================
*/
std::shared_ptr<idAFBody> Physics_PlayerChain::GetBody(const int id) const {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		gameLocal.Error("GetBody: no body with id %d exists\n", id);
		return nullptr;
	}
	return bodies[id];
}

/*
================
Physics_PlayerChain::DeleteBody
================
*/
void Physics_PlayerChain::DeleteBody(const int id) {
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
Physics_PlayerChain::Evaluate
================
*/
bool Physics_PlayerChain::Evaluate(int timeStepMSec, int endTimeMSec) noexcept {

	Physics_PlayerChain::MovePlayer(timeStepMSec);

	float timeStep;

	timeStep = MS2SEC(timeStepMSec);

	// check for collisions between current and next state
	CheckForCollisions(timeStep);

	// swap the current and next state
	SwapStates();

	// make sure all clip models are disabled in case they were enabled for self collision
	/*if (selfCollision) {
		DisableClip();
	}*/
	EnableClip();

	// apply collision impulses
	ApplyCollisions(timeStep);

	ActivateContactEntities();

	if (IsOutsideWorld()) {
		gameLocal.Warning("Physics_PlayerChain outside world bounds for entity '%s' type '%s' at (%s)",
			self->name.c_str(), self->GetType()->classname.c_str(),
			GetOrigin().ToString(0).c_str());
	}

	return true;
}

/*
================
Physics_PlayerChain::Evolve
================
*/
void Physics_PlayerChain::Evolve(float timeStep) noexcept {
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
Physics_PlayerChain::CollisionImpulse

  apply impulse to the colliding bodies
  the current state of the body should be set to the moment of impact
  this is silly as it doesn't take the AF structure into account
================
*/
bool Physics_PlayerChain::CollisionImpulse(float timeStep, idAFBody* body, trace_t& collision) noexcept {
	auto ent = gameLocal.entities[collision.c.entityNum];
	if (ent.get() == self) {
		return false;
	}

	auto velocity = body->current->spatialVelocity;

	// callback to self to let the entity know about the impact
	return self->Collide(collision, velocity);
}

/*
================
Physics_PlayerChain::ApplyCollisions
================
*/
bool Physics_PlayerChain::ApplyCollisions(float timeStep) noexcept {
	for (size_t i = 0; i < collisions.size(); i++) {
		if (CollisionImpulse(timeStep, collisions[i].body.get(), collisions[i].trace)) {
			return true;
		}
	}
	return false;
}

/*
================
Physics_PlayerChain::SetupCollisionForBody
================
*/
std::shared_ptr<idEntity> Physics_PlayerChain::SetupCollisionForBody(idAFBody* body) const noexcept {
	size_t i;
	std::shared_ptr<idEntity> passEntity;

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
Physics_PlayerChain::CheckForCollisions

  check for collisions between the current and next state
  if there is a collision the next state is set to the state at the moment of impact
  assumes all bodies are linked for collision detection and relinks all bodies after moving them
================
*/
void Physics_PlayerChain::CheckForCollisions(float timeStep) {
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
				body->clipModel.get(), body->clipMask, passEntity.get())) {

				// set the next state to the state at the moment of impact
				body->next->worldOrigin = collision.endpos;

				// add collision to the list
				auto index = collisions.size();
				collisions.resize(index + 1);
				collisions[index].trace = collision;
				collisions[index].body = body;
			}
		}

		body->clipModel->Link(gameLocal.clip, self, body->clipModel->GetId(), body->next->worldOrigin);
	}

	MoveEachBodiesToPrevOne();
}

void Physics_PlayerChain::MoveEachBodiesToPrevOne() {
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

		bodies[i]->clipModel->Link(gameLocal.clip, self, bodies[i]->clipModel->GetId(), bodies[i]->next->worldOrigin);
	}
}

/*
================
Physics_PlayerChain::SwapStates
================
*/
void Physics_PlayerChain::SwapStates() noexcept {
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
Physics_PlayerChain::UpdateClipModels
================
*/
void Physics_PlayerChain::UpdateClipModels() {
	for (size_t i = 0; i < bodies.size(); i++) {
		auto body = bodies[i];
		body->clipModel->Link(gameLocal.clip, self, body->clipModel->GetId(), body->current->worldOrigin);
	}
}

/*
================
Physics_PlayerChain::IsAtRest
================
*/
bool Physics_PlayerChain::IsAtRest() const noexcept {
	return false;
}

/*
================
Physics_PlayerChain::GetNumClipModels
================
*/
int Physics_PlayerChain::GetNumClipModels() const noexcept {
	return bodies.size();
}

/*
================
Physics_PlayerChain::GetBounds
================
*/
const idBounds& Physics_PlayerChain::GetBounds(int id) const noexcept {
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
Physics_PlayerChain::GetAbsBounds
================
*/
const idBounds& Physics_PlayerChain::GetAbsBounds(int id) const noexcept {
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
Physics_PlayerChain::SaveState
================
*/
void Physics_PlayerChain::SaveState() noexcept {
	saved = current;

	for (size_t i = 0; i < bodies.size(); i++) {
		memcpy(&bodies[i]->saved, bodies[i]->current, sizeof(AFBodyPState_t));
	}
}

/*
================
Physics_PlayerChain::RestoreState
================
*/
void Physics_PlayerChain::RestoreState() noexcept {
	current = saved;

	for (size_t i = 0; i < bodies.size(); i++) {
		*(bodies[i]->current) = bodies[i]->saved;
	}

	EvaluateContacts();
}

/*
================
Physics_PlayerChain::SetOrigin
================
*/
void Physics_PlayerChain::SetOrigin(const Vector2& newOrigin, int id) noexcept {
	Translate(newOrigin - bodies[0]->current->worldOrigin);
}

/*
================
Physics_PlayerChain::GetOrigin
================
*/
const Vector2& Physics_PlayerChain::PlayerGetOrigin() const noexcept {
	return current.origin;
}

/*
================
Physics_PlayerChain::Translate
================
*/
void Physics_PlayerChain::Translate(const Vector2& translation, int id) noexcept {
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
const Vector2& Physics_PlayerChain::GetOrigin(int id) const noexcept {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return vec2_origin;
	}
	else {
		return bodies[id]->current->worldOrigin;
	}
}

/*
================
Physics_PlayerChain::SetLinearVelocity
================
*/
void Physics_PlayerChain::SetLinearVelocity(const Vector2& newLinearVelocity, int id) noexcept {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return;
	}
	bodies[id]->current->spatialVelocity = newLinearVelocity;
	Activate();
}

/*
================
Physics_PlayerChain::GetLinearVelocity
================
*/
const Vector2& Physics_PlayerChain::GetLinearVelocity(int id) const noexcept {
	if (id < 0 || id >= static_cast<int>(bodies.size())) {
		return vec2_origin;
	}
	else {
		return bodies[id]->current->spatialVelocity;
	}
}

/*
================
Physics_PlayerChain::DisableClip
================
*/
void Physics_PlayerChain::DisableClip() noexcept {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Disable();
	}
}

/*
================
Physics_PlayerChain::EnableClip
================
*/
void Physics_PlayerChain::EnableClip() noexcept {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Enable();
	}
}

/*
================
Physics_PlayerChain::UnlinkClip
================
*/
void Physics_PlayerChain::UnlinkClip() noexcept {
	for (size_t i = 0; i < bodies.size(); i++) {
		bodies[i]->clipModel->Unlink();
	}
}

/*
================
Physics_PlayerChain::LinkClip
================
*/
void Physics_PlayerChain::LinkClip() noexcept {
	UpdateClipModels();
}