
#include "idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

void R_ClearEntityDefDynamicModel(idRenderEntityLocal* def) {
  // clear the dynamic model if present
  if (def->dynamicModel) {
    // this is copied from cachedDynamicModel, so it doesn't need to be freed
    def->dynamicModel = nullptr;
  }
  def->dynamicModelFrameCount = 0;
}

bool R_IssueEntityDefCallback(idRenderEntityLocal* def) {
  bool update;
  if (tr.viewDef) {
    update = def->parms.callback(&def->parms, &tr.viewDef->renderView);
  } else {
    update = def->parms.callback(&def->parms, nullptr);
  }

  if (def->parms.hModel == NULL) {
    common->Error(
        "R_IssueEntityDefCallback: dynamic entity callback didn't set model");
  }

  return update;
}

idRenderModel* R_EntityDefDynamicModel(idRenderEntityLocal* def) {
  /*if (def->dynamicModelFrameCount == tr.frameCount) {
          return def->dynamicModel;
  }*/

  // allow deferred entities to construct themselves
  bool callbackUpdate;
  if (def->parms.callback) {
    callbackUpdate = R_IssueEntityDefCallback(def);
  } else {
    callbackUpdate = false;
  }

  auto model = def->parms.hModel;

  if (!model) {
    common->Error("R_EntityDefDynamicModel: NULL model");
    return NULL;
  }

  if (model->IsDynamicModel() == DM_STATIC) {
    def->dynamicModel = nullptr;
    // def->dynamicModelFrameCount = 0;
    return model;
  }

  // continously animating models (particle systems, etc) will have their
  // snapshot updated every single view
  if (callbackUpdate || (model->IsDynamicModel() == DM_CONTINUOUS &&
                         def->dynamicModelFrameCount != tr.frameCount)) {
    R_ClearEntityDefDynamicModel(def);
  }

  // if we don't have a snapshot of the dynamic model, generate it now
  if (!def->dynamicModel) {
    // instantiate the snapshot of the dynamic model, possibly reusing memory
    // from the cached snapshot
    model->InstantiateDynamicModel(&def->parms, tr.viewDef.get(),
                                   def->cachedDynamicModel);

    def->dynamicModel = def->cachedDynamicModel.get();
    // def->dynamicModelFrameCount = tr.frameCount;
  }

  return def->dynamicModel;
}

void R_AddSingleModel(const viewEntity_t& vEntity) {
  // globals we really should pass in...
  const auto viewDef = tr.viewDef;

  auto entityDef = vEntity.entityDef;
  const auto renderEntity = &entityDef->parms;
  // const idRenderWorldLocal * world = entityDef->world;

  // if the entity wasn't seen through a portal chain, it was added just for
  // light shadows
  const bool modelIsVisible = true;  // !vEntity->scissorRect.IsEmpty();

  // if we aren't visible and none of the shadows stretch into the view,
  // we don't need to do anything else
  if (!modelIsVisible) {
    return;
  }

  //---------------------------
  // create a dynamic model if the geometry isn't static
  //---------------------------
  auto model = R_EntityDefDynamicModel(entityDef);
  if (!model || model->NumSurfaces() <= 0) {
    return;
  }

  //---------------------------
  // add all the model surfaces
  //---------------------------

  const auto model_col = renderEntity->color;
  const auto modelNumSurfaces = model->NumSurfaces();

  auto rEntSp = vEntity.entityDef;

  if (rEntSp) {
    const auto curPos = /*Vector2(static_cast<float>(tr.borderHeight),
                           static_cast<float>(tr.borderWidth)) +*/
        rEntSp->parms.origin.GetIntegerVectorFloor();

    for (int surfaceNum = 0; surfaceNum != modelNumSurfaces; surfaceNum++) {
      const ModelPixel& surf = model->Surface(surfaceNum);
      if (surf.screenPixel.color == colorNone) continue;
      tr.screen->set(curPos + surf.origin, surf.screenPixel);
    }
  }
}

void R_AddModels() {
  // tr.viewDef->viewEntitys = R_SortViewEntities(tr.viewDef->viewEntitys);

  //-------------------------------------------------
  // Go through each view entity that is either visible to the view, or to
  // any light that intersects the view (for shadows).
  //-------------------------------------------------

  for (auto vEntity = tr.viewDef->viewEntitys; vEntity;
       vEntity = vEntity->next) {
    R_AddSingleModel(*vEntity);
  }
}
