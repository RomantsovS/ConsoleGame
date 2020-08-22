#include "Common_local.h"
#include "..//d3xp/Game.h"
#include "../renderer/tr_local.h"
#include "UsercmdGen.h"
#include "EventLoop.h"

void idCommonLocal::Frame()
{
	/*unsigned key = 0;

	if (tr.screen.readInput(key))
	{
		std::string text_input;

		switch (key)
		{
		case 27:
			tr.screen.setStdOutputBuffer();

			tr.screen.writeConsoleOutput("enter Q to quit or any key to continue: ");

			text_input = tr.screen.waitConsoleInput();

			if (text_input == "Q" || text_input == "q")
			{
				Quit();
			}
			else if (text_input == "listEntities")
			{
				Cmd_EntityList_f();
			}
		default:
			;
		}
		tr.screen.clearConsoleOutut();
		tr.screen.setDrawOutputBuffer();
	}*/

	try
	{
		const bool pauseGame = !mapSpawned;

		eventLoop->RunEventLoop();

		if (!mapSpawned)
			ExecuteMapChange();

		usercmdGen->BuildCurrentUsercmd(0);

		if (pauseGame) {
			usercmdGen->Clear();
		}

		usercmd_t newCmd = usercmdGen->GetCurrentUsercmd();

		Draw();
	}
	catch (std::exception& err)
	{
		common->Error(err.what());
	}
}

void idCommonLocal::Draw()
{
	if (game)
	{
		game->RunFrame();
		
		game->Draw(0);

		RB_DrawView();
	}
}