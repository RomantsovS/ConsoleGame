#include "Common_local.h"
#include "..//d3xp/Game.h"
#include "../renderer/tr_local.h"

void idCommonLocal::Draw()
{
	if (game)
	{
		game->RunFrame();
		game->Draw(0);

		RB_DrawView();
	}
}