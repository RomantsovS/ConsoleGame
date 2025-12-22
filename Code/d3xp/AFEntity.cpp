#include "idlib/precompiled.h"

#include "Game_local.h"

/*
===============================================================================

  idMultiModelAF

===============================================================================
*/

CLASS_DECLARATION(idEntity, idMultiModelAF)
END_CLASS

/*
================
idMultiModelAF::Spawn
================
*/
void idMultiModelAF::Spawn() {
  physicsObj = std::make_shared<idPhysics_AF>();
  physicsObj->SetSelf(this);
}

/*
================
idMultiModelAF::~idMultiModelAF
================
*/
idMultiModelAF::~idMultiModelAF() {
  for (size_t i = 0; i < modelDefHandles.size(); i++) {
    if (modelDefHandles[i] != -1) {
      gameRenderWorld->FreeEntityDef(modelDefHandles[i]);
      modelDefHandles[i] = -1;
    }
  }
}

/*
================
idMultiModelAF::SetModelForId
================
*/
void idMultiModelAF::SetModelForId(int id, const std::string& modelName) {
  modelHandles.resize(id + 1);
  modelDefHandles.resize(id + 1, -1);
  modelHandles[id] = renderModelManager->FindModel(modelName);
}

/*
================
idMultiModelAF::Present
================
*/
void idMultiModelAF::Present() {
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
================
idMultiModelAF::Think
================
*/
void idMultiModelAF::Think() {
  RunPhysics();
  Present();
}

void idMultiModelAF::Remove() noexcept {
  for (size_t i = physicsObj->GetNumClipModels(); i > 0; --i) {
    physicsObj->DeleteBody(i - 1);
  }

  physicsObj = nullptr;
  idEntity::Remove();
}

bool idMultiModelAF::Collide(const trace_t& collision,
                             const Vector2& velocity) noexcept {
  if (collision.c.entityNum == ENTITYNUM_WORLD) {
    return true;
  } else {
    auto ent = gameLocal.entities[collision.c.entityNum];

    if (!ent->IsActive()) return true;
  }

  return false;
}

/*
===============================================================================

  idChain

===============================================================================
*/

CLASS_DECLARATION(idMultiModelAF, idChain)
END_CLASS

/*
================
idChain::BuildChain

  builds a chain hanging down from the ceiling
  the highest link is a child of the link below it etc.
  this allows an object to be attached to multiple chains while keeping a single
tree structure
================
*/
void idChain::BuildChain(const std::string& name, const Vector2& origin,
                         float linkLength, int numLinks, const Vector2& dir) {
  int i;
  std::shared_ptr<idAFBody> body, lastBody;
  Vector2 org;

  idTraceModel trm;

  idBounds bounds;
  Vector2 size;

  spawnArgs.GetVector("size", "", size);

  if ((size.x < 0.0f) || (size.y < 0.0f)) {
    gameLocal.Error("Invalid size '%s' on entity '%s'", size.ToString(),
                    name.c_str());
  }
  bounds[0].Set(size.x * -0.5f, size.y * -0.5f);
  bounds[1].Set(size.x * 0.5f, size.y * 0.5f);

  trm.SetupBox(bounds);

  org = origin;

  for (i = 0; i < numLinks; i++) {
    // add body
    auto clip = std::make_shared<idClipModel>(trm);
    clip->SetContents(static_cast<int>(contentsFlags_t::CONTENTS_SOLID));
    clip->Link(gameLocal.clip, this, i, org);
    body = std::make_shared<idAFBody>(name + std::to_string(i), clip);
    physicsObj->AddBody(body);

    // visual model for body
    SetModelForId(physicsObj->GetBodyId(body), spawnArgs.GetString("model"));

    org += dir;

    lastBody = body;
  }
}

/*
================
idChain::Spawn
================
*/
void idChain::Spawn() {
  int numLinks;
  Vector2 origin;

  spawnArgs.GetInt("links", "3", numLinks);
  origin = GetPhysics()->GetOrigin();

  // initialize physics
  physicsObj->SetSelf(this);
  physicsObj->SetClipMask(MASK_SOLID |
                          static_cast<int>(contentsFlags_t::CONTENTS_BODY));
  SetPhysics(physicsObj);

  Vector2 linearVelocity;
  spawnArgs.GetVector("linearVelocity", "0 10", linearVelocity);

  Vector2 size;
  spawnArgs.GetVector("size", "", size);

  Vector2 dir = vec2_origin;
  if (linearVelocity.x > 0)
    dir.x = -size.x;
  else if (linearVelocity.x < 0)
    dir.x = size.x;

  if (linearVelocity.y > 0)
    dir.y = -size.y;
  else if (linearVelocity.y < 0)
    dir.y = size.y;

  BuildChain("link", origin, 1.0f, numLinks, dir);

  physicsObj->SetLinearVelocity(linearVelocity, 0);
}