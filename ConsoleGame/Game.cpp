#include "Game.h"
#include "Vector2.h"
#include "Model_local.h"

void GameEdit::ParseSpawnArgsToRenderEntity(const Dict * args, renderEntity_s * renderEntity)
{
	renderEntity->hModel = new RenderModelStatic;

	args->GetVector("origin", "0 0", renderEntity->pos);
}
