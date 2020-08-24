#include "Common_local.h"
#include "../d3xp/Game_local.h"
#include "../renderer/tr_local.h"
#include "FileSystem.h"
#include "KeyInput.h"
#include "CmdSystem.h"
#include "UsercmdGen.h"
#include "EventLoop.h"
#include "Console.h"

idCVar com_allowConsole("com_allowConsole", "1", CVAR_BOOL | CVAR_SYSTEM | CVAR_INIT, "allow toggling console with the tilde key");

idCVar com_showFPS("com_showFPS", "1", CVAR_BOOL | CVAR_SYSTEM | CVAR_ARCHIVE | CVAR_NOCHEAT, "show frames rendered per second");

idCVar com_engineHz("com_engineHz", "60", CVAR_FLOAT | CVAR_ARCHIVE, "Frames per second the engine runs at", 10.0f, 1024.0f);
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

	gameFrame = 0;
	gameTimeResidual = 0;
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
	try {
		// init console command system
		cmdSystem->Init();

		// init CVar system
		cvarSystem->Init();

		// register all static CVars
		idCVar::RegisterStaticVars();

		// initialize key input/binding, done early so bind command exists
		idKeyInput::Init();

		// init the console so we can take prints
		console->Init();

		// initialize the file system
		fileSystem->Init();

		// init journalling, etc
		eventLoop->Init();

		// exec the startup scripts
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec default.cfg\n");

		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec autoexec.cfg\n");

		// run cfg execution
		cmdSystem->ExecuteCommandBuffer();

		com_engineHz_denominator = 100LL * static_cast<long long>(com_engineHz.GetFloat());

		// initialize the renderSystem data structures
		renderSystem->Init();

		// init the user command input code
		usercmdGen->Init();

		game->Init();

		// the same idRenderWorld will be used for all games
		// and demos, insuring that level specific models
		// will be freed
		renderWorld = renderSystem->AllocRenderWorld();

		delayMilliseconds = 100;
		FPSupdateMilliseconds = 1000;

		Printf("--- Common Initialization Complete ---\n");
	}
	catch (std::exception& msg) {
		Sys_Error("Error during initialization %s", msg.what());
	}
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

	// shut down the user command input code
	printf("usercmdGen->Shutdown();\n");
	usercmdGen->Shutdown();

	// shut down the event loop
	printf("eventLoop->Shutdown();\n");
	eventLoop->Shutdown();

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

	// shut down the console
	printf("console->Shutdown();\n");
	console->Shutdown();

	// shut down the key system
	printf("idKeyInput::Shutdown();\n");
	idKeyInput::Shutdown();

	// shut down the cvar system
	printf("cvarSystem->Shutdown();\n");
	cvarSystem->Shutdown();

	// shut down the console command system
	printf("cmdSystem->Shutdown();\n");
	cmdSystem->Shutdown();
}

void idCommonLocal::Stop(bool resetSession)
{
	// clear mapSpawned and demo playing flags
	UnloadMap();
}

/*
===============
idCommonLocal::ProcessEvent
===============
*/
bool idCommonLocal::ProcessEvent(const sysEvent_t* event) {
	// hitting escape anywhere brings up the menu
	if (game && game->IsInGame()) {
		if (event->evType == SE_KEY && event->evValue2 == 1 &&
			(event->evValue == static_cast<int>(keyNum_t::K_ESCAPE))) {
			if (!game->Shell_IsActive()) {

				// menus / etc
				if (MenuEvent(event)) {
					return true;
				}

				console->Close();

				StartMenu();
				return true;
			}
			else {
				console->Close();

				// menus / etc
				if (MenuEvent(event)) {
					return true;
				}
			}
		}
	}

	// let the pull-down console take it if desired
	if (console->ProcessEvent(event, false)) {
		return true;
	}

	// menus / etc
	if (MenuEvent(event)) {
		return true;
	}

	// if we aren't in a game, force the console to take it
	if (!mapSpawned) {
		console->ProcessEvent(event, true);
		return true;
	}

	// in game, exec bindings for all key downs
	if (event->evType == SE_KEY && event->evValue2 == 1) {
		idKeyInput::ExecKeyBinding(event->evValue);
		return true;
	}

	return false;
}
