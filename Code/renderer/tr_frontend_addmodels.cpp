
#include "idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

idRenderModel* R_EntityDefDynamicModel(idRenderEntityLocal* def) {
  /*if (def->dynamicModelFrameCount == tr.frameCount) {
          return def->dynamicModel;
  }*/

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

  // if we don't have a snapshot of the dynamic model, generate it now
  if (!def->dynamicModel) {
    // instantiate the snapshot of the dynamic model, possibly reusing memory
    // from the cached snapshot
    def->cachedDynamicModel = model->InstantiateDynamicModel(
        &def->parms, tr.viewDef.get(), def->cachedDynamicModel);

    def->dynamicModel = def->cachedDynamicModel;
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

  /*if (r_useParallelAddModels.GetBool()) {
          for (viewEntity_t * vEntity = tr.viewDef->viewEntitys; vEntity !=
  NULL; vEntity = vEntity->next) {
                  tr.frontEndJobList->AddJob((jobRun_t)R_AddSingleModel,
  vEntity);
          }
          tr.frontEndJobList->Submit();
          tr.frontEndJobList->Wait();
  }
  else {*/
  for (auto vEntity = tr.viewDef->viewEntitys; vEntity;
       vEntity = vEntity->next) {
    R_AddSingleModel(*vEntity);
  }
  //}
}