#include "idlib/precompiled.h"

#include "Game_local.h"
#include "../framework/Common_local.h"

CLASS_DECLARATION(idPlayer, PlayerChain)
END_CLASS

PlayerChain::PlayerChain() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "PlayerChain");
#endif  // DEBUG_PRINT_Ctor_Dtor
}

PlayerChain::~PlayerChain() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s dtor\n", "PlayerChain");
#endif  // DEBUG_PRINT_Ctor_Dtor
}

/*
================
PlayerChain::SetModelForId
================
*/
void PlayerChain::SetModelForId(int id, const std::string& modelName) {
  modelHandles.resize(id + 1);
  modelDefHandles.resize(id + 1, -1);
  modelHandles[id] = renderModelManager->FindModel(modelName);
}

/*
================
idMultiModelAF::Present
================
*/
void PlayerChain::Present() {
  // don't present to the renderer if the entity hasn't changed
  if (!(thinkFlags & TH_UPDATEVISUALS)) {
    return;
  }
  BecomeInactive(TH_UPDATEVISUALS);

  for (size_t i = 0; i < modelHandles.size(); i++) {
    if (!modelHandles[i]) {
      continue;
    }

    renderEntity.origin = physicsObj->GetOrigin(i);
    renderEntity.axis = physicsObj->GetAxis(i);
    renderEntity.hModel = modelHandles[i].get();

    // add to refresh list
    if (modelDefHandles[i] == -1) {
      modelDefHandles[i] = gameRenderWorld->AddEntityDef(&renderEntity);
    } else {
      gameRenderWorld->UpdateEntityDef(modelDefHandles[i], &renderEntity);
    }
  }
}

/*
==============
PlayerChain::Init
==============
*/
void PlayerChain::Init() {
  int numLinks;
  Vector2 origin;

  spawnArgs.GetInt("links", "3", numLinks);
  origin = GetPhysics()->GetOrigin();

  Vector2 linearVelocity;
  spawnArgs.GetVector("linearVelocity", "0 10", linearVelocity);
  auto size = spawnArgs.GetVector("size", "1 1");

  Vector2 dir = vec2_origin;

  for (size_t i = 0; i < 2; ++i) {
    if (linearVelocity[i] > 0)
      dir[i] = -size.x;
    else if (linearVelocity[i] < 0)
      dir[i] = size.x;
  }

  BuildChain("link", origin, 1.0f, numLinks, dir);
}

/*
================
PlayerChain::BuildChain

  builds a chain hanging down from the ceiling
  the highest link is a child of the link below it etc.
  this allows an object to be attached to multiple chains while keeping a single
tree structure
================
*/
void PlayerChain::BuildChain(const std::string& name, const Vector2& origin,
                             float linkLength, int numLinks,
                             const Vector2& dir) {
  int i;
  std::shared_ptr<idAFBody> body, lastBody;
  Vector2 org;

  idTraceModel trm;
  float density = 0.0f;
  std::string clipModelName;

  // check if a clip model is set
  spawnArgs.GetString("clipmodel", "", &clipModelName);
  if (!clipModelName[0]) {
    clipModelName = spawnArgs.GetString("model");  // use the visual model
  }

  /*if (!collisionModelManager->TrmFromModel(clipModelName, trm)) {
          gameLocal.Error("idSimpleObject '%s': cannot load collision model %s",
  name, clipModelName); return;
  }*/

  org = origin;

  for (i = 0; i < numLinks; i++) {
    // AddModel(trm, org, i, density);
    AddModel(org, i, density);

    org += dir;

    lastBody = body;
  }
}

void PlayerChain::AddModel(const idTraceModel& trm, const Vector2& origin,
                           const int id, const float density) {
  // add body
  auto clip = std::make_shared<idClipModel>(trm);
  // clip->SetContents(CONTENTS_SOLID);
  clip->Link(gameLocal.clip, this, id, origin);
  auto body =
      std::make_shared<idAFBody>(name + std::to_string(id), clip, density);
  physicsObj->AddBody(body);

  // visual model for body
  SetModelForId(physicsObj->GetBodyId(body), spawnArgs.GetString("model"));
}

void PlayerChain::AddModel(const Vector2& origin, const int id,
                           const float density) {
  // add body
  auto clip = std::make_shared<idClipModel>(*GetPhysics()->GetClipModel());
  // clip->SetContents(CONTENTS_SOLID);
  clip->Link(gameLocal.clip, this, id, origin);
  auto body =
      std::make_shared<idAFBody>(name + std::to_string(id), clip, density);
  physicsObj->AddBody(body);

  // visual model for body
  SetModelForId(physicsObj->GetBodyId(body), spawnArgs.GetString("model"));
}

/*
==============
PlayerChain::Spawn

Prepare any resources used by the player.
==============
*/
void PlayerChain::Spawn() {
  // set our collision model
  physicsObj = std::make_shared<Physics_PlayerChain>();
  physicsObj->SetSelf(this);
  SetClipModel();
  // physicsObj.SetMass(spawnArgs.GetFloat("mass", "100"));
  physicsObj->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_BODY));
  physicsObj->SetClipMask(MASK_PLAYERSOLID);
  SetPhysics(physicsObj);

  // init the damage effects
  playerView.SetPlayerEntity(this);

  if (common->IsMultiplayer()) {
    Init();
    if (!common->IsClient()) {
      // set yourself ready to spawn. idMultiplayerGame will decide when/if
      // appropriate and call SpawnFromSpawnSpot
      //SpawnFromSpawnSpot();
    }
  } else {
    //SpawnFromSpawnSpot();
  }
}

/*
==============
PlayerChain::Collide
==============
*/
bool PlayerChain::Collide(const trace_t& collision,
                          const Vector2& velocity) noexcept {
  auto other = gameLocal.entities[collision.c.entityNum];
  if (other) {
    if (other->IsType(idSimpleObject::Type) || other->IsType(idChain::Type)) {
      idTraceModel trm;
      float density = 0.0f;
      std::string clipModelName;

      // check if a clip model is set
      spawnArgs.GetString("clipmodel", "", &clipModelName);
      if (!clipModelName[0]) {
        clipModelName = spawnArgs.GetString("model");  // use the visual model
      }

      if (!collisionModelManager->TrmFromModel(clipModelName, trm)) {
        gameLocal.Error("PlayerChain '%s': cannot load collision model %s",
                        name, clipModelName);
        return true;
      }

      auto last_body =
          physicsObj->GetBody(GetPhysics()->GetNumClipModels() - 1);
      AddModel(trm, last_body->GetClipModel()->GetOrigin(),
               GetPhysics()->GetNumClipModels(), density);

      physicsObj->EnableClip();

      other->PostEventMS(&EV_Remove, 0);
      gameLocal.AddRandomPoint();

      return true;
    } else if (other->IsType(idChain::Type)) {
      other->PostEventMS(&EV_Remove, 0);
      return true;
    } else if (other->IsType(idStaticEntity::Type)) {
    }
  }
  return false;
}

/*
==============
idPlayer::SetClipModel
==============
*/
void PlayerChain::SetClipModel() {
  physicsObj->SetClipModel(GetPhysics()->GetClipModel(), 1.0f);
}

/*
==============
PlayerChain::Think

Called every tic for each player
==============
*/
void PlayerChain::Think() {
  /*playedTimeResidual += (gameLocal.time - gameLocal.previousTime);
  playedTimeSecs += playedTimeResidual / 1000;
  playedTimeResidual = playedTimeResidual % 1000;

  buttonMask &= usercmd.buttons;
  usercmd.buttons &= ~buttonMask;*/

  EvaluateControls();

  Move();

  Present();

  if (!(thinkFlags & TH_THINK)) {
    gameLocal.Printf("player %d not thinking?\n", entityNumber);
  }
}

/*
==============
PlayerChain::EvaluateControls
==============
*/
void PlayerChain::EvaluateControls() noexcept {
  /*if (usercmd.impulseSequence != oldImpulseSequence) {
          PerformImpulse(usercmd.impulse);
  }

  oldImpulseSequence = usercmd.impulseSequence;*/

  AdjustSpeed();
}

/*
==============
PlayerChain::AdjustSpeed
==============
*/
void PlayerChain::AdjustSpeed() noexcept {
  float speed{};

  if (usercmd.buttons & BUTTON_RUN) {
    speed = pm_runspeed.GetFloat();
  } else {
    speed = pm_walkspeed.GetFloat();
  }

  physicsObj->SetSpeed(speed, 0.0f);
}

/*
==============
PlayerChain::Move
==============
*/
void PlayerChain::Move() {
  Vector2 oldOrigin;
  Vector2 oldVelocity;
  // Vector2 pushVelocity;

  // save old origin and velocity for crashlanding
  oldOrigin = physicsObj->GetOrigin();
  oldVelocity = physicsObj->GetLinearVelocity();
  // pushVelocity = physicsObj->GetPushedLinearVelocity();

  // physicsObj.SetContents(CONTENTS_BODY);
  // physicsObj.SetMovementType(PM_NORMAL);

  physicsObj->SetClipMask(MASK_SOLID);

  {
    // Vector2	org;
    // idMat3	axis;
    // GetViewPos(org, axis);

    physicsObj->SetPlayerInput(usercmd, vec2_origin);
  }

  // FIXME: physics gets disabled somehow
  BecomeActive(TH_PHYSICS);
  RunPhysics();
}