#include "CollisionModel_local.h"
#include "../framework/Common_local.h"

void idCollisionModelManagerLocal::Translation(trace_t* results, const Vector2& start, const Vector2& end,
	const std::shared_ptr<idTraceModel> trm, int contentMask, int model, const Vector2& modelOrigin)
{
	if (model < 0 || model > MAX_SUBMODELS || model > idCollisionModelManagerLocal::maxModels) {
		common->Printf("idCollisionModelManagerLocal::Translation: invalid model handle\n");
		return;
	}
	if (!idCollisionModelManagerLocal::models[model]) {
		common->Printf("idCollisionModelManagerLocal::Translation: invalid model\n");
		return;
	}


}