#pragma hdrstop
#include <precompiled.h>

#include "../Game_local.h"

CLASS_DECLARATION(idPhysics_PlayerBase, Physics_PlayerMy)
END_CLASS

/*
============
idPhysics_Player::CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
float Physics_PlayerMy::CmdScale(const usercmd_t& cmd) const {
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
	scale = GetPlayerSpeed() * max / (127.0f * total);

	return scale;
}

/*
==================
idPhysics_Player::SlideMove

Returns true if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5

bool Physics_PlayerMy::SlideMove(bool gravity, bool stepUp, bool stepDown, bool push) {
	float		time_left;
	Vector2		end;
	trace_t		trace;

	time_left = GetFrameTime();

	// calculate position we are trying to move to
	end = current.origin + time_left * current.velocity;

	if (auto selfSp = self.lock()) {
		// see if we can make it there
		if (gameLocal.clip.Translation(trace, current.origin, end, clipModel.get(), clipMask, selfSp.get())) {
			// let the entity know about the collision
			selfSp->Collide(trace, current.velocity);
		}
	}

	time_left -= time_left * trace.fraction;
	current.origin = trace.endpos;

	return true;
}

/*
==================
Physics_PlayerMy::Friction

Handles both ground friction and water friction
==================
*/
void Physics_PlayerMy::Friction() {
}

/*
===================
Physics_PlayerMy::WalkMove
===================
*/
void Physics_PlayerMy::WalkMove() {
	Physics_PlayerMy::Friction();

	float scale = CmdScale(GetUserCmd());
	Vector2 wishvel{};

	if (GetUserCmd().forwardmove > 0) {
		wishvel += Vector2(0.0f, GetPlayerSpeed());
	}
	else if (GetUserCmd().forwardmove < 0) {
		wishvel += Vector2(0.0f, -GetPlayerSpeed());
	}
	
	if (GetUserCmd().rightmove > 0) {
		wishvel += Vector2(GetPlayerSpeed(), 0.0f);
	}
	else if (GetUserCmd().rightmove < 0) {
		wishvel += Vector2(-GetPlayerSpeed(), 0.0f);
	}

	current.velocity = wishvel;

	SlideMove(false, true, true, true);
}

/*
================
Physics_PlayerMy::MovePlayer
================
*/
void Physics_PlayerMy::MovePlayer(int msec) {

	idPhysics_PlayerBase::MovePlayer(msec);

	// move the player velocity into the frame of a pusher
	//current.velocity -= current.pushVelocity;

	// walking on ground
	Physics_PlayerMy::WalkMove();

	// move the player velocity back into the world frame
	//current.velocity += current.pushVelocity;
	//current.pushVelocity.Zero();
}

/*
================
Physics_PlayerMy::Physics_PlayerMy
================
*/
Physics_PlayerMy::Physics_PlayerMy() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "Physics_PlayerMy");
#endif // DEBUG_PRINT_Ctor_Dtor
	clipModel = nullptr;
	clipMask = 0;
	memset(&current, 0, sizeof(current));
	saved = current;

	contacts.clear();
}

/*
================
Physics_PlayerMy::~Physics_PlayerMy
================
*/
Physics_PlayerMy::~Physics_PlayerMy() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s dtor\n", "Physics_PlayerMy");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
================
Physics_PlayerMy::Evaluate
================
*/
bool Physics_PlayerMy::Evaluate(int timeStepMSec, int endTimeMSec) {

	clipModel->Unlink();

	ActivateContactEntities();

	Physics_PlayerMy::MovePlayer(timeStepMSec);

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);

	if (IsOutsideWorld()) {
		gameLocal.Warning("Physics_PlayerMy outside world bounds for entity '%s' type '%s' at (%s)",
			self.lock()->name.c_str(), self.lock()->GetType()->classname.c_str(),
			GetOrigin().ToString(0).c_str());
	}

	return true;
}

/*
================
Physics_PlayerMy::IsAtRest
================
*/
bool Physics_PlayerMy::IsAtRest() const {
	return false;
}

/*
================
Physics_PlayerMy::SaveState
================
*/
void Physics_PlayerMy::SaveState() {
	saved = current;
}

/*
================
Physics_PlayerMy::RestoreState
================
*/
void Physics_PlayerMy::RestoreState() {
	current = saved;

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);

	EvaluateContacts();
}

/*
================
Physics_PlayerMy::SetOrigin
================
*/
void Physics_PlayerMy::SetOrigin(const Vector2& newOrigin, int id) {
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
Physics_PlayerMy::GetOrigin
================
*/
const Vector2& Physics_PlayerMy::PlayerGetOrigin() const {
	return current.origin;
}

/*
================
Physics_PlayerMy::Translate
================
*/
void Physics_PlayerMy::Translate(const Vector2& translation, int id) {
	current.localOrigin += translation;
	current.origin += translation;

	clipModel->Link(gameLocal.clip, self.lock(), 0, current.origin);
}

/*
================
Physics_PlayerMy::SetLinearVelocity
================
*/
void Physics_PlayerMy::SetLinearVelocity(const Vector2& newLinearVelocity, int id) {
	current.velocity = newLinearVelocity;
}

/*
================
Physics_PlayerMy::GetLinearVelocity
================
*/
const Vector2& Physics_PlayerMy::GetLinearVelocity(int id) const {
	return current.velocity;
}