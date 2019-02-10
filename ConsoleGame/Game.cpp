#include "Game.h"
#include "Vector2.h"
#include "Model_local.h"

void GameEdit::ParseSpawnArgsToRenderEntity(const Dict * args, renderEntity_s * renderEntity)
{
	renderEntity->hModel = new RenderModelStatic;

	args->GetVector("origin", "0 0", renderEntity->origin);
	args->GetVector("axis", "0 0", renderEntity->axis);

	renderEntity->hModel->SetColor(static_cast<Screen::ConsoleColor>(args->GetInt("color", 15)));
}
