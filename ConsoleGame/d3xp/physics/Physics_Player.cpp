#pragma hdrstop
#include "../../idlib/precompiled.h"

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_Actor, idPhysics_Player)
END_CLASS

/*
==================
idPhysics_Player::SlideMove

Returns true if the velocity was clipped in some way
==================
*/
constexpr size_t MAX_CLIP_PLANES = 5;

/*
================
idPhysics_Player::idPhysics_Player
================
*/
idPhysics_Player::idPhysics_Player() {
	clipModel = nullptr;
	clipMask = 0;
	memset(&command, 0, sizeof(command));
}

/*
================
Physics_PlayerChain::SetSpeed
================
*/
void idPhysics_Player::SetSpeed(const float newWalkSpeed, const float newCrouchSpeed) {
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
idPhysics_Player::Evaluate
================
*/
bool idPhysics_Player::Evaluate(int timeStepMSec, int endTimeMSec) {
	return false;
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
}

/*
================
idPhysics_Player::RestoreState
================
*/
void idPhysics_Player::RestoreState() {
}

/*
================
idPhysics_Player::SetOrigin
================
*/
void idPhysics_Player::SetOrigin(const Vector2& newOrigin, int id) {
}

/*
================
idPhysics_Player::GetOrigin
================
*/
const Vector2& idPhysics_Player::PlayerGetOrigin() const {
	return vec2_origin;
}

/*
================
idPhysics_Player::Translate
================
*/
void idPhysics_Player::Translate(const Vector2& translation, int id) {
}

/*
================
idPhysics_Player::SetLinearVelocity
================
*/
void idPhysics_Player::SetLinearVelocity(const Vector2& newLinearVelocity, int id) {
}

/*
================
idPhysics_Player::GetLinearVelocity
================
*/
const Vector2& idPhysics_Player::GetLinearVelocity(int id) const {
	return vec2_origin;
}