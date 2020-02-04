#include <conio.h>
#include <iostream>

#include "Common_local.h"
#include "Game_local.h"
#include "tr_local.h"

long long com_engineHz_numerator = 100LL * 1000LL;
long long com_engineHz_denominator = 100LL * 60LL;

idCommonLocal commonLocal;
idCommon * common = &commonLocal;

idCommonLocal::idCommonLocal()
{
	renderWorld = nullptr;
}

/*
==================
idCommonLocal::Quit
==================
*/
void idCommonLocal::Quit()
{

	// don't try to shutdown if we are in a recursive error
	if (!com_errorEntered)
	{
		Shutdown();
	}
}

void idCommonLocal::Init(int argc, const char * const * argv, const char * cmdline)
{
	game->Init();

	// initialize the renderSystem data structures
	renderSystem->Init();

	// the same idRenderWorld will be used for all games
	// and demos, insuring that level specific models
	// will be freed
	renderWorld = renderSystem->AllocRenderWorld();

	delayMilliseconds = 100;
	FPSupdateMilliseconds = 1000;

	gameRunning = true;
}

/*
=================
idCommonLocal::Shutdown
=================
*/
void idCommonLocal::Shutdown() {

	if (com_shuttingDown)
	{
		return;
	}
	com_shuttingDown = true;

	// shut down the renderSystem
	//printf("renderSystem->Shutdown();\n");
	renderSystem->Shutdown();

	// shut down the game object
	if (game)
	{
		game->Shutdown();
	}

	renderWorld = nullptr;
}

void idCommonLocal::Frame()
{
	const bool pauseGame = !mapSpawned;

	if (!mapSpawned)
		ExecuteMapChange();

	char c = 0;

	//tr.ClearScreen();

	if (_kbhit())
	{
		c = _getch();

		switch (c)
		{
		case 27:
			gameRunning = false;

			tr.ClearScreen();

			std::cout << "enter Q to quit or any key to continue: ";

			std::cin >> c;

			while (std::cin.get() != '\n')
				continue;

			if (c == 'Q' || c == 'q')
			{
				gameRunning = false;

				return;
			}

			gameRunning = true;
		default:
			//onKeyPressed(c);
			;
		}
	}

	try
	{
		Draw();
	}
	catch (std::exception &err)
	{
		gameRunning = false;
		std::cout << err.what() << std::endl
			<< "press ane key to continue...\n";
		_getch();
	}
}
