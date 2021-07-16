#pragma hdrstop
#include <precompiled.h>

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_Actor, idPhysics_PlayerBase)
END_CLASS

/*
============
idPhysics_Player::CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
float idPhysics_PlayerBase::CmdScale(const usercmd_t& cmd) const {
	int		max;
	float	total;
	float	scale;

	int forwardmove = cmd.forwardmove;
	int rightmove = cmd.rightmove;
	//int upmove = 0;

	// since the crouch key doubles as downward movement, ignore downward movement when we're on the ground
	// otherwise crouch speed will be lower than specified
	/*if (!walking) {
		upmove = ((cmd.buttons & BUTTON_JUMP) ? 127 : 0) - ((cmd.buttons & BUTTON_CROUCH) ? 127 : 0);
	}*/

	max = abs(forwardmove);
	if (abs(rightmove) > max) {
		max = abs(rightmove);
	}
	/*if (abs(upmove) > max) {
		max = abs(upmove);
	}*/

	if (!max) {
		return 0.0f;
	}

	total = idMath::Sqrt((float)forwardmove * forwardmove + rightmove * rightmove/* + upmove * upmove*/);
	scale = (float)playerSpeed * max / (127.0f * total);

	return scale;
}

/*
==================
idPhysics_PlayerBase::SlideMove

Returns true if the velocity was clipped in some way
==================
*/
constexpr size_t MAX_CLIP_PLANES = 5;

/*
================
idPhysics_PlayerBase::idPhysics_PlayerBase
================
*/
idPhysics_PlayerBase::idPhysics_PlayerBase() {
	clipModel = nullptr;
	clipMask = 0;
	memset(&command, 0, sizeof(command));
}

/*
================
Physics_PlayerChain::SetSpeed
================
*/
void idPhysics_PlayerBase::SetSpeed(const float newWalkSpeed, const float newCrouchSpeed) {
	walkSpeed = newWalkSpeed;
}

/*
================
idPhysics_PlayerBase::SetPlayerInput
================
*/
void idPhysics_PlayerBase::SetPlayerInput(const usercmd_t& cmd, const Vector2& forwardVector) {
	command = cmd;
}

/*
================
idPhysics_PlayerBase::Evaluate
================
*/
bool idPhysics_PlayerBase::Evaluate(int timeStepMSec, int endTimeMSec) {
	return false;
}

/*
================
idPhysics_PlayerBase::IsAtRest
================
*/
bool idPhysics_PlayerBase::IsAtRest() const {
	return false;
}

/*
================
idPhysics_PlayerBase::SaveState
================
*/
void idPhysics_PlayerBase::SaveState() {
}

/*
================
idPhysics_PlayerBase::RestoreState
================
*/
void idPhysics_PlayerBase::RestoreState() {
}

/*
================
idPhysics_PlayerBase::SetOrigin
================
*/
void idPhysics_PlayerBase::SetOrigin(const Vector2& newOrigin, int id) {
}

/*
================
idPhysics_PlayerBase::GetOrigin
================
*/
const Vector2& idPhysics_PlayerBase::PlayerGetOrigin() const {
	return vec2_origin;
}

/*
================
idPhysics_PlayerBase::Translate
================
*/
void idPhysics_PlayerBase::Translate(const Vector2& translation, int id) {
}

/*
================
idPhysics_PlayerBase::SetLinearVelocity
================
*/
void idPhysics_PlayerBase::SetLinearVelocity(const Vector2& newLinearVelocity, int id) {
}

/*
================
idPhysics_PlayerBase::GetLinearVelocity
================
*/
const Vector2& idPhysics_PlayerBase::GetLinearVelocity(int id) const {
	return vec2_origin;
}

/*
================
idPhysics_PlayerBase::MovePlayer
================
*/
void idPhysics_PlayerBase::MovePlayer(int msec) {

	// determine the time
	framemsec = msec;
	frametime = framemsec * 0.001f;

	// default speed
	playerSpeed = walkSpeed;
}