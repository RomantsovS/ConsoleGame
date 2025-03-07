#include "idlib/precompiled.h"

#include "tr_local.h"
#include "Model_local.h"

void idRenderModelPrt::InitFromFile(const std::string& fileName) {
  name = fileName;
  particleSystem = std::static_pointer_cast<idDeclParticle>(
      declManager->FindType(declType_t::DECL_PARTICLE, fileName));
}

idRenderModel* idRenderModelPrt::InstantiateDynamicModel(
    const renderEntity_t* renderEntity, const viewDef_t* viewDef,
    idRenderModel* cachedModel) {
  idRenderModelStatic* staticModel;

  // this may be triggered by a model trace or other non-view related source, to
  // which we should look like an empty model
  if (!renderEntity || !viewDef) {
    delete cachedModel;
    return NULL;
  }

  if (cachedModel != NULL) {
    assert(dynamic_cast<idRenderModelStatic*>(cachedModel) != NULL);
    staticModel = static_cast<idRenderModelStatic*>(cachedModel);
  } else {
    staticModel = new idRenderModelStatic;
    staticModel->InitEmpty("");
  }

  particleGen_t g;

  g.renderEnt = renderEntity;
  g.renderView = &viewDef->renderView;

  for (int stageNum = 0; stageNum < particleSystem->stages.size(); stageNum++) {
    auto stage = particleSystem->stages[stageNum];

    if (!stage->material) {
      continue;
    }
    if (!stage->cycleMsec) {
      continue;
    }

    int stageAge = g.renderView->time[0] - stage->timeOffset * 1000;
    int stageCycle = stageAge / stage->cycleMsec;
    /*
    int surfaceNum;
    modelSurface_t* surf;

    if (staticModel->FindSurfaceWithId(stageNum, surfaceNum)) {
      surf = &staticModel->surfaces[surfaceNum];
      R_FreeStaticTriSurfVertexCaches(surf->geometry);
    } else {
      surf = &staticModel->surfaces.Alloc();
      surf->id = stageNum;
      surf->shader = stage->material;
      surf->geometry = R_AllocStaticTriSurf();
      R_AllocStaticTriSurfVerts(surf->geometry, 4 * count);
      R_AllocStaticTriSurfIndexes(surf->geometry, 6 * count);
    }

    for (int index = 0; index < stage->totalParticles; index++) {
      g.index = index;

      int particleAge = stageAge - bunchOffset;
      int particleCycle = particleAge / stage->cycleMsec;
      if (particleCycle < 0) {
        // before the particleSystem spawned
        continue;
      }
      if (stage->cycles && particleCycle >= stage->cycles) {
        // cycled systems will only run cycle times
        continue;
      }

      int inCycleTime = particleAge - particleCycle * stage->cycleMsec;

      // supress particles before or after the age clamp
      g.frac = (float)inCycleTime / (stage->particleLife * 1000);
      if (g.frac < 0.0f) {
        // yet to be spawned
        continue;
      }
      if (g.frac > 1.0f) {
        // this particle is in the deadTime band
        continue;
      }

      g.age = g.frac * stage->particleLife;

      // if the particle doesn't get drawn because it is faded out or beyond a
      // kill region, don't increment the verts
      numVerts += stage->CreateParticle(&g);
    }
    */
  }

  return staticModel;
}

dynamicModel_t idRenderModelPrt::IsDynamicModel() const noexcept {
  return DM_CONTINUOUS;
}
