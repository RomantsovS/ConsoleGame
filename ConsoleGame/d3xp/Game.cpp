#include "Game.h"
#include "../idlib/math/Vector2.h"
#include "../renderer/Model_local.h"
#include "../renderer/ModelManager.h"

void idGameEdit::ParseSpawnArgsToRenderEntity(const idDict * args, renderEntity_t * renderEntity)
{
	auto temp = args->GetString("model");

	if (!temp.empty())
	{
		if (!renderEntity->hModel) {
			renderEntity->hModel = renderModelManager->FindModel(temp);
		}
	}

	args->GetVector("origin", "0 0", renderEntity->origin);
	args->GetVector("axis", "0 0", renderEntity->axis);
}
