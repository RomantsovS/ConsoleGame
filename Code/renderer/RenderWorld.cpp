#include "idlib/precompiled.h"

#include "tr_local.h"

idRenderWorldLocal::idRenderWorldLocal() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s ctor\n", "idRenderWorldLocal");
#endif  // DEBUG_PRINT_Ctor_Dtor

  mapName.clear();

  areaNodes.clear();
  numAreaNodes = 0;

  portalAreas.clear();
  numPortalAreas = 0;
}

idRenderWorldLocal::~idRenderWorldLocal() {
#ifdef DEBUG_PRINT_Ctor_Dtor
  common->DPrintf("%s dtor\n", "idRenderWorldLocal");
#endif  // DEBUG_PRINT_Ctor_Dtor

  // free up the debug lines, polys, and text
  RB_ClearDebugLines(0);
  RB_ClearDebugText(0);
}

int idRenderWorldLocal::AddEntityDef(const renderEntity_t* re) {
  // try and reuse a free spot
  int entityHandle;

  auto iter = std::find(entityDefs.begin(), entityDefs.end(), nullptr);

  if (iter == entityDefs.end()) {
    entityHandle = entityDefs.size();
    entityDefs.resize(entityDefs.size() + 4);
  } else
    entityHandle = iter - entityDefs.begin();

  UpdateEntityDef(entityHandle, re);

  return entityHandle;
}

void idRenderWorldLocal::UpdateEntityDef(
    int entityHandle, gsl::not_null<const renderEntity_t*> re) {
  if (!re->hModel && !re->callback) {
    common->Error("idRenderWorld::UpdateEntityDef: NULL hModel");
  }

  // create new slots if needed
  if (entityHandle < 0 || entityHandle > LUDICROUS_INDEX) {
    common->Error("idRenderWorld::UpdateEntityDef: index = %i", entityHandle);
  }
  while (entityHandle >= static_cast<int>(entityDefs.size())) {
    entityDefs.resize(entityDefs.size() + 4);
  }

  std::shared_ptr<idRenderEntityLocal>& def = entityDefs[entityHandle];

  if (def) {
    /* if (!re->forceUpdate) {
      // check for exact match (OPTIMIZE: check through pointers more)
      if (!def->dynamicModel &&
          !memcmp(re, &def->parms, sizeof(*re))) {
        return;
      }

      // if the only thing that changed was shaderparms, we can just leave
      // things as they are after updating parms

      // if we have a callback function and the bounds, origin, axis and model
      // match, then we can leave the references as they are
      if (re->callback) {
        bool axisMatch = (re->axis == def->parms.axis);
        bool originMatch = (re->origin == def->parms.origin);
        bool modelMatch = (re->hModel == def->parms.hModel);

        if (boundsMatch && originMatch && axisMatch && modelMatch) {
          // only clear the dynamic model and interaction surfaces if they exist
          R_ClearEntityDefDynamicModel(def);
          def->parms = *re;
          return;
        }
      }
    }*/
    // save any decals if the model is the same, allowing marks to move with
    // entities
    if (def->parms.hModel == re->hModel) {
      R_FreeEntityDefDerivedData(def.get(), true, true);
    } else {
      R_FreeEntityDefDerivedData(def.get(), false, false);
    }
  } else {
    // creating a new one
    def = std::make_shared<idRenderEntityLocal>();

    entityDefs[entityHandle] = def;

    def->world = this;
    def->index = entityHandle;
  }

  def->parms = *re;

  // optionally immediately issue any callbacks
  /*if (!r_useEntityCallbacks.GetBool() &&  def->parms.callback) {
    R_IssueEntityDefCallback(def);
  }*/

  // based on the model bounds, add references in each area
  // that may contain the updated surface
  R_CreateEntityRefs(def.get());
}

/*
===================
FreeEntityDef

Frees all references and lit surfaces from the model, and
NULL's out it's entry in the world list
===================
*/
void idRenderWorldLocal::FreeEntityDef(int entityHandle) {
  if (entityHandle < 0 || entityHandle >= static_cast<int>(entityDefs.size())) {
    common->Printf("idRenderWorld::FreeEntityDef: handle %i > %i\n",
                   entityHandle, entityDefs.size());
    return;
  }

  auto def = entityDefs[entityHandle];
  if (!def) {
    common->Printf("idRenderWorld::FreeEntityDef: handle %i is NULL\n",
                   entityHandle);
    return;
  }

  R_FreeEntityDefDerivedData(def.get(), false, false);

  // if we are playing a demo, these will have been freed
  // in R_FreeEntityDefDerivedData(), otherwise the gui
  // object still exists in the game

  entityDefs[entityHandle] = nullptr;
}

const renderEntity_t* idRenderWorldLocal::GetRenderEntity(
    int entityHandle) const {
  if (entityHandle < 0 || entityHandle >= static_cast<int>(entityDefs.size())) {
    common->Printf(
        "idRenderWorld::GetRenderEntity: invalid handle %i [0, %i]\n",
        entityHandle, entityDefs.size());
    return nullptr;
  }

  auto def = entityDefs[entityHandle];
  if (!def) {
    common->Printf("idRenderWorld::GetRenderEntity: handle %i is NULL\n",
                   entityHandle);
    return nullptr;
  }

  return &def->parms;
}

void idRenderWorldLocal::RenderScene(const renderView_t* renderView) {
  if (!tr.update_frame) return;

  auto parms = std::make_shared<viewDef_t>();
  parms->renderView = *renderView;
  parms->renderWorld = std::dynamic_pointer_cast<idRenderWorldLocal>(getptr());

  R_RenderView(parms);
}

void idRenderWorldLocal::AddEntityRefToArea(idRenderEntityLocal* def,
                                            portalArea_t* area) {
  if (!def) {
    common->Error("idRenderWorldLocal::AddEntityRefToArea: NULL def");
    return;
  }

  for (auto ref = def->entityRefs; ref; ref = ref->ownerNext) {
    if (ref->area == area) {
      return;
    }
  }

  auto ref = std::make_shared<areaReference_t>();

  ref->entity = def;

  // link to entityDef
  ref->ownerNext = def->entityRefs;
  def->entityRefs = ref;

  // link to end of area list
  ref->area = area;
  ref->areaNext = area->entityRefs;
  ref->areaPrev = area->entityRefs->areaPrev;
  ref->areaNext->areaPrev = ref;
  ref->areaPrev->areaNext = ref;
}

void idRenderWorldLocal::PushFrustumIntoTree_r(idRenderEntityLocal* def,
                                               int nodeNum) {
  if (nodeNum < 0) {
    int areaNum = -1 - nodeNum;
    auto area = &portalAreas[areaNum];
    if (area->viewCount == tr.viewCount) {
      return;  // already added a reference here
    }
    area->viewCount = tr.viewCount;

    if (def) {
      AddEntityRefToArea(def, area);
    }

    return;
  }

  auto node = &areaNodes.at(nodeNum);

  nodeNum = node->children[0];
  if (nodeNum != 0) {  // 0 = solid
    PushFrustumIntoTree_r(def, nodeNum);
  }

  nodeNum = node->children[1];
  if (nodeNum != 0) {  // 0 = solid
    PushFrustumIntoTree_r(def, nodeNum);
  }
}

void idRenderWorldLocal::PushFrustumIntoTree(idRenderEntityLocal* def) {
  if (areaNodes.empty()) {
    return;
  }

  PushFrustumIntoTree_r(def, 0);
}

/*
====================
idRenderWorldLocal::DebugClearLines
====================
*/
void idRenderWorldLocal::DebugClearLines(int time) {
  RB_ClearDebugLines(time);
  RB_ClearDebugText(time);
}

/*
================
idRenderWorldLocal::DrawText

  oriented on the viewaxis
  align can be 0-left, 1-center (default), 2-right
================
*/
void idRenderWorldLocal::DrawTextToScreen(std::string text,
                                          const Vector2& origin,
                                          const Screen::color_type color,
                                          const int lifetime = 0) {
  RB_AddDebugText(std::move(text), origin, color, lifetime);
}

void idRenderWorldLocal::DebugLine(const Screen::color_type color,
                                   const Vector2& start, const Vector2& end,
                                   const int lifetime,
                                   const bool depthTest) noexcept {
  RB_AddDebugLine(color, start, end, lifetime, depthTest);
}

void idRenderWorldLocal::DebugBounds(const Screen::color_type color,
                                     const idBounds& bounds, const Vector2& org,
                                     const int lifetime) {
  int i;
  Vector2 v[4];

  if (bounds.IsCleared()) {
    return;
  }

  for (i = 0; i < 4; i++) {
    v[i][0] = org[0] + bounds[(i ^ (i >> 1)) & 1][0];
    v[i][1] = org[1] + bounds[(i >> 1) & 1][1];
    // v[i][2] = org[2] + bounds[(i >> 2) & 1][2];
  }

  DebugLine(color, v[0], v[1], lifetime);
  DebugLine(color, v[1], v[2], lifetime);
  DebugLine(color, v[0], v[3], lifetime);
  DebugLine(color, v[3], v[2], lifetime);
}

std::shared_ptr<idRenderWorld> idRenderWorldLocal::getptr() {
  return shared_from_this();
}
