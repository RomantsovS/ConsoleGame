
#include "idlib/precompiled.h"

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
float Physics_PlayerMy::CmdScale(const usercmd_t& cmd) const noexcept {
  int max;
  float total;
  float scale;

  int forwardmove = cmd.forwardmove;
  int rightmove = cmd.rightmove;
  // int upmove = 0;

  // since the crouch key doubles as downward movement, ignore downward movement
  // when we're on the ground otherwise crouch speed will be lower than
  // specified
  /*if (!walking) {
          upmove = ((cmd.buttons & BUTTON_JUMP) ? 127 : 0) - ((cmd.buttons &
  BUTTON_CROUCH) ? 127 : 0);
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

  total = idMath::Sqrt((float)forwardmove * forwardmove +
                       rightmove * rightmove /* + upmove * upmove*/);
  scale = GetPlayerSpeed() * max / (127.0f * total);

  return scale;
}

/*
==================
idPhysics_Player::SlideMove

Returns true if the velocity was clipped in some way
==================
*/
#define MAX_CLIP_PLANES 5

bool Physics_PlayerMy::SlideMove(bool gravity, bool stepUp, bool stepDown,
                                 bool push) {
  size_t bumpcount, numbumps = 3;
  float time_left;
  Vector2 end, clipVelocity = current.velocity;
  trace_t trace;
  bool stepped = false;

  time_left = GetFrameTime();

  std::vector<int> collidedEntities;

  for (bumpcount = 0; bumpcount < numbumps; bumpcount++) {
    // calculate position we are trying to move to
    end = current.origin + time_left * clipVelocity;

    // see if we can make it there
    gameLocal.clip.Translation(trace, current.origin, end, clipModel.get(),
                               clipMask, self);

    time_left -= time_left * trace.fraction;
    current.origin = trace.endpos;

    // if moved the entire distance
    if (trace.fraction >= 1.0f) {
      stepped = true;
      break;
    }

    if (!stepped) {
      if (std::find(std::begin(collidedEntities), std::end(collidedEntities),
                    trace.c.entityNum) == std::end(collidedEntities)) {
        // let the entity know about the collision
        self->Collide(trace, current.velocity);
        collidedEntities.push_back(trace.c.entityNum);
      }
    }

    if (bumpcount == 0 && current.velocity.x != 0.0f)
      clipVelocity = {current.velocity.x, 0.0f};
    else if (current.velocity.y != 0.0f)
      clipVelocity = {0.0f, current.velocity.y};
    else
      break;
  }

  return bumpcount == 0;
}

/*
==================
Physics_PlayerMy::Friction

Handles both ground friction and water friction
==================
*/
void Physics_PlayerMy::Friction() noexcept {}

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
  } else if (GetUserCmd().forwardmove < 0) {
    wishvel += Vector2(0.0f, -GetPlayerSpeed());
  }

  if (GetUserCmd().rightmove > 0) {
    wishvel += Vector2(GetPlayerSpeed(), 0.0f);
  } else if (GetUserCmd().rightmove < 0) {
    wishvel += Vector2(-GetPlayerSpeed(), 0.0f);
  }

  current.velocity = wishvel;

  SlideMove(false, true, true, true);
}

/*
=============
idPhysics_Player::CheckGround
=============
*/
void Physics_PlayerMy::CheckGround() {
  // set the clip model origin before getting the contacts
  // clipModel->SetPosition(current.origin, clipModel->GetAxis());

  EvaluateContacts();
}

/*
================
Physics_PlayerMy::MovePlayer
================
*/
void Physics_PlayerMy::MovePlayer(int msec) {
  idPhysics_PlayerBase::MovePlayer(msec);

  // move the player velocity into the frame of a pusher
  // current.velocity -= current.pushVelocity;

  // no control when dead
  if (current.movementType == pmtype_t::PM_DEAD) {
    command.forwardmove = 0;
    command.rightmove = 0;
    command.buttons &= ~(BUTTON_JUMP | BUTTON_CROUCH);
  }

  // check for ground
  CheckGround();

  // move
  if (current.movementType == pmtype_t::PM_DEAD) {
    // dead
    // idPhysics_PlayerBase::DeadMove();
  } else {
    // walking on ground
    WalkMove();
  }

  // move the player velocity back into the world frame
  // current.velocity += current.pushVelocity;
  // current.pushVelocity.Zero();
}

/*
================
Physics_PlayerMy::Physics_PlayerMy
================
*/
Physics_PlayerMy::Physics_PlayerMy() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "Physics_PlayerMy");
#endif  // DEBUG_PRINT_Ctor_Dtor
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
#endif  // DEBUG_PRINT_Ctor_Dtor
}

/*
================
Physics_PlayerMy::Evaluate
================
*/
bool Physics_PlayerMy::Evaluate(int timeStepMSec, int endTimeMSec) noexcept {
  clipModel->Unlink();

  ActivateContactEntities();

  Physics_PlayerMy::MovePlayer(timeStepMSec);

  clipModel->Link(gameLocal.clip, self, 0, current.origin);

  if (IsOutsideWorld()) {
    gameLocal.Warning(
        "Physics_PlayerMy outside world bounds for entity '%s' type '%s' at "
        "(%s)",
        self->name.c_str(), self->GetType()->classname.c_str(),
        GetOrigin().ToString(0).c_str());
  }

  return current.origin != saved.origin;
}

/*
================
Physics_PlayerMy::IsAtRest
================
*/
bool Physics_PlayerMy::IsAtRest() const noexcept { return false; }

/*
================
Physics_PlayerMy::SaveState
================
*/
void Physics_PlayerMy::SaveState() noexcept { saved = current; }

/*
================
Physics_PlayerMy::RestoreState
================
*/
void Physics_PlayerMy::RestoreState() noexcept {
  current = saved;

  clipModel->Link(gameLocal.clip, self, 0, current.origin);

  EvaluateContacts();
}

/*
================
Physics_PlayerMy::SetOrigin
================
*/
void Physics_PlayerMy::SetOrigin(const Vector2& newOrigin, int id) noexcept {
  current.localOrigin = newOrigin;
  /*if (masterEntity) {
          self->GetMasterPosition(masterOrigin, masterAxis);
          current.origin = masterOrigin + newOrigin * masterAxis;
  }
  else {*/
  current.origin = newOrigin;
  //}

  clipModel->Link(gameLocal.clip, self, 0, newOrigin);
}

void Physics_PlayerMy::SetMovementType(const pmtype_t type) {
  current.movementType = type;
}

/*
================
Physics_PlayerMy::GetOrigin
================
*/
const Vector2& Physics_PlayerMy::PlayerGetOrigin() const noexcept {
  return current.origin;
}

/*
================
Physics_PlayerMy::Translate
================
*/
void Physics_PlayerMy::Translate(const Vector2& translation, int id) noexcept {
  current.localOrigin += translation;
  current.origin += translation;

  clipModel->Link(gameLocal.clip, self, 0, current.origin);
}

/*
================
Physics_PlayerMy::SetLinearVelocity
================
*/
void Physics_PlayerMy::SetLinearVelocity(const Vector2& newLinearVelocity,
                                         int id) noexcept {
  current.velocity = newLinearVelocity;
}

/*
================
Physics_PlayerMy::GetLinearVelocity
================
*/
const Vector2& Physics_PlayerMy::GetLinearVelocity(int id) const noexcept {
  return current.velocity;
}

/*
================
Physics_PlayerMy::WriteToSnapshot
================
*/
void Physics_PlayerMy::WriteToSnapshot(idBitMsg& msg) const {
  msg.WriteFloat(current.origin[0]);
  msg.WriteFloat(current.origin[1]);
  msg.WriteFloat(current.velocity[0]);
  msg.WriteFloat(current.velocity[1]);
}

/*
================
Physics_PlayerMy::ReadFromSnapshot
================
*/
void Physics_PlayerMy::ReadFromSnapshot(const idBitMsg& msg) {
  saved = current;

  current.origin[0] = msg.ReadFloat();
  current.origin[1] = msg.ReadFloat();
  current.velocity[0] = msg.ReadFloat();
  current.velocity[1] = msg.ReadFloat();

  if (clipModel) {
    clipModel->Link(gameLocal.clip, self, 0, current.origin);
  }
}