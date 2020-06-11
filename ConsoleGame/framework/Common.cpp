//#include <conio.h>
//#include <iostream>

#include "Common_local.h"
#include "../d3xp/Game_local.h"
#include "../renderer/tr_local.h"
#include "FileSystem.h"
#include "../d3xp/CmdSystem.h"

long long com_engineHz_numerator = 100LL * 1000LL;
long long com_engineHz_denominator = 100LL * 60LL;

idCommonLocal commonLocal;
idCommon * common = &commonLocal;

idCommonLocal::idCommonLocal()
{
	com_errorEntered = ERP_NONE;
	com_shuttingDown = false;

	logFile = nullptr;

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
	Sys_Quit();
}

void idCommonLocal::Init(int argc, const char * const * argv, const char * cmdline)
{
	// initialize the file system
	fileSystem->Init();

	game->Init();

	// initialize the renderSystem data structures
	renderSystem->Init();

	// the same idRenderWorld will be used for all games
	// and demos, insuring that level specific models
	// will be freed
	renderWorld = renderSystem->AllocRenderWorld();

	delayMilliseconds = 100;
	FPSupdateMilliseconds = 1000;
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

	Printf("Stop();\n");
	Stop();

	Printf("delete renderWorld;\n");
	renderWorld = nullptr;

	// shut down the renderSystem
	Printf("renderSystem->Shutdown();\n");
	renderSystem->Shutdown();

	// unload the game dll
	Printf("UnloadGameDLL();\n");
	// shut down the game object
	if (game)
	{
		game->Shutdown();
	}

	// only shut down the log file after all output is done
	Printf("CloseLogFile();\n");
	CloseLogFile();

	// shut down the file system
	printf("fileSystem->Shutdown( false );\n");
	fileSystem->Shutdown(false);
}

void idCommonLocal::Stop(bool resetSession)
{
	// clear mapSpawned and demo playing flags
	UnloadMap();
}

void idCommonLocal::Frame()
{
	const bool pauseGame = !mapSpawned;

	if (!mapSpawned)
		ExecuteMapChange();

	unsigned key = 0;
	std::string text_input;

	if (tr.screen.readInput(key))
	{
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
	}

	try
	{
		Draw();
	}
	catch (std::exception &err)
	{
		common->Error(err.what());
	}
}
