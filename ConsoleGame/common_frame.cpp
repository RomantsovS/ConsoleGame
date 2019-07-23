#include "Common_local.h"
#include "Game.h"

void idCommonLocal::Draw()
{
	if (game)
	{
		game->RunFrame();
		game->Draw(0);
	}
}