#include "Physics_Player.h"
#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_Actor, idPhysics_Player)

/*
==================
idPhysics_Player::SlideMove

Returns true if the velocity was clipped in some way
==================
*/
constexpr size_t MAX_CLIP_PLANES = 5;

bool idPhysics_Player::SlideMove() {
	int			bumpcount, numbumps;
	float		time_left;
	Vector2		end;
	trace_t		trace;

	numbumps = 1;

	time_left = frametime;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {

		// calculate position we are trying to move to
		end = current.origin + time_left * current.velocity;

		// see if we can make it there
		gameLocal.clip->Translation(trace, current.origin, end, clipModel, clipMask, self.lock());

		time_left -= time_left * trace.fraction;
		current.origin = trace.endpos;

		// if moved the entire distance
		if (trace.fraction >= 1.0f) {
			break;
		}

		if (true) {
			// let the entity know about the collision
			self.lock()->Collide(trace, current.velocity);
		}
	}

	return (bool)(bumpcount == 0);
}

/*
==================
idPhysics_Player::Friction

Handles both ground friction and water friction
==================
*/
void idPhysics_Player::Friction() {
}

/*
===================
idPhysics_Player::WalkMove
===================
*/
void idPhysics_Player::WalkMove() {
	idPhysics_Player::Friction();

	if (command.forwardmove > 0) {
		this->SetLinearVelocity(Vector2(1.0f, 0.0f));
	}
	else if (command.forwardmove < 0) {
		this->SetLinearVelocity(Vector2(-1.0f, 0.0f));
	}
	else if (command.rightmove > 0) {
		this->SetLinearVelocity(Vector2(0.0f, 1.0f));
	}
	else if (command.rightmove < 0) {
		this->SetLinearVelocity(Vector2(0.0f, -1.0f));
	}

	idPhysics_Player::SlideMove();
}

/*
================
idPhysics_Player::MovePlayer
================
*/
void idPhysics_Player::MovePlayer(int msec) {

	// determine the time
	framemsec = msec;
	frametime = framemsec * 0.001f;

	// move the player velocity into the frame of a pusher
	//current.velocity -= current.pushVelocity;

	// walking on ground
	idPhysics_Player::WalkMove();

	// move the player velocity back into the world frame
	//current.velocity += current.pushVelocity;
	//current.pushVelocity.Zero();
}

/*
================
idPhysics_Player::idPhysics_Player
================
*/
idPhysics_Player::idPhysics_Player() {
	clipModel = nullptr;
	clipMask = 0;
	memset(&current, 0, sizeof(current));
	saved = current;
	walkSpeed = 0;
	memset(&command, 0, sizeof(command));
	framemsec = 0;
	frametime = 0;
}

/*
================
idPhysics_Player::SetPlayerInput
================
*/
void idPhysics_Player::SetPlayerInput(const usercmd_t& cmd, const Vector2& forwardVector) {
	command = cmd;
}

/*
================
idPhysics_Player::SetSpeed
================
*/
void idPhysics_Player::SetSpeed(const float newWalkSpeed, const float newCrouchSpeed) {
	walkSpeed = newWalkSpeed;
	//crouchSpeed = newCrouchSpeed;
}

/*
================
idPhysics_Player::Evaluate
================
*/
bool idPhysics_Player::Evaluate(int timeStepMSec, int endTimeMSec) {
	clipModel->Unlink();

	ActivateContactEntities();

	idPhysics_Player::MovePlayer(timeStepMSec);

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);

	if (IsOutsideWorld()) {
		gameLocal.Warning("clip model outside world bounds for entity '%s' at (%s)", self.lock()->name.c_str(), current.origin.ToString(0));
	}

	return true; //( current.origin != oldOrigin );
}

/*
================
idPhysics_Player::IsAtRest
================
*/
bool idPhysics_Player::IsAtRest() const {
	return false;
}

/*
================
idPhysics_Player::SaveState
================
*/
void idPhysics_Player::SaveState() {
	saved = current;
}

/*
================
idPhysics_Player::RestoreState
================
*/
void idPhysics_Player::RestoreState() {
	current = saved;

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);

	EvaluateContacts();
}

/*
================
idPhysics_Player::SetOrigin
================
*/
void idPhysics_Player::SetOrigin(const Vector2& newOrigin, int id) {
	current.localOrigin = newOrigin;
	/*if (masterEntity) {
		self->GetMasterPosition(masterOrigin, masterAxis);
		current.origin = masterOrigin + newOrigin * masterAxis;
	}
	else {*/
		current.origin = newOrigin;
	//}

	clipModel->Link(gameLocal.clip, self.lock(), 0, newOrigin);
}

/*
================
idPhysics_Player::GetOrigin
================
*/
const Vector2& idPhysics_Player::PlayerGetOrigin() const {
	return current.origin;
}

/*
================
idPhysics_Player::Translate
================
*/
void idPhysics_Player::Translate(const Vector2& translation, int id) {

	current.localOrigin += translation;
	current.origin += translation;

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);
}

/*
================
idPhysics_Player::SetLinearVelocity
================
*/
void idPhysics_Player::SetLinearVelocity(const Vector2& newLinearVelocity, int id) {
	current.velocity = newLinearVelocity;
}

/*
================
idPhysics_Player::GetLinearVelocity
================
*/
const Vector2& idPhysics_Player::GetLinearVelocity(int id) const {
	return current.velocity;
}