#include "tr_local.h"
#include "Model_local.h"

std::shared_ptr<idRenderModel> R_EntityDefDynamicModel(std::shared_ptr<idRenderEntityLocal> def)
{
	auto model = def->parms.hModel;

	if (model->IsDynamicModel() == DM_STATIC)
	{
		return model;
	}

	return model;
}

void R_AddSingleModel(std::shared_ptr<viewEntity_t> vEntity)
{
	// globals we really should pass in...
	const auto viewDef = tr.viewDef;

	auto entityDef = vEntity->entityDef;
	const auto renderEntity = &entityDef->parms;
	//const idRenderWorldLocal * world = entityDef->world;

	// if the entity wasn't seen through a portal chain, it was added just for light shadows
	const bool modelIsVisible = true;// !vEntity->scissorRect.IsEmpty();

	// if we aren't visible and none of the shadows stretch into the view,
	// we don't need to do anything else
	if (!modelIsVisible) {
		return;
	}

	//---------------------------
	// create a dynamic model if the geometry isn't static
	//---------------------------
	auto model = R_EntityDefDynamicModel(entityDef);
	if (!model || model->NumSurfaces() <= 0)
	{
		return;
	}

	//---------------------------
	// add all the model surfaces
	//---------------------------
	for (int surfaceNum = 0; surfaceNum < model->NumSurfaces(); surfaceNum++)
	{
		const ModelPixel & surf = model->Surface(surfaceNum);

		tr.screen.set(Vector2(tr.borderHeight, tr.borderWidth) + vEntity->entityDef->parms.origin + surf.origin, surf.screenPixel);
	}
}

void R_AddModels()
{
	//tr.viewDef->viewEntitys = R_SortViewEntities(tr.viewDef->viewEntitys);

	//-------------------------------------------------
	// Go through each view entity that is either visible to the view, or to
	// any light that intersects the view (for shadows).
	//-------------------------------------------------

	/*if (r_useParallelAddModels.GetBool()) {
		for (viewEntity_t * vEntity = tr.viewDef->viewEntitys; vEntity != NULL; vEntity = vEntity->next) {
			tr.frontEndJobList->AddJob((jobRun_t)R_AddSingleModel, vEntity);
		}
		tr.frontEndJobList->Submit();
		tr.frontEndJobList->Wait();
	}
	else {*/
		for (auto vEntity = tr.viewDef->viewEntitys; vEntity; vEntity = vEntity->next) {
			R_AddSingleModel(vEntity);
		}
	//}
}