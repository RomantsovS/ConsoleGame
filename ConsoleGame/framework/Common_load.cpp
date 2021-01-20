#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Common_local.h"

/*
===============
idCommonLocal::StartNewGame
===============
*/
void idCommonLocal::StartNewGame(const std::string& mapName, bool devmap, int gameMode) {
	if (!session->IsLocalUserRegistered()) {
		// For development make sure a controller is registered
		// Can't just register the local user because it will be removed because of it's persistent state
	}

	std::string mapNameClean = mapName;

	session->QuitMatchToTitle();
	if (WaitForSessionState(idSession::sessionState_t::IDLE)) {
		session->StartMatch();
	}
}

/*
===============
idCommonLocal::ExecuteMapChange

Performs the initialization of a game based on session match parameters, used for both single
player and multiplayer, but not for renderDemos, which don't create a game at all.
Exits with mapSpawned = true
===============
*/
void idCommonLocal::ExecuteMapChange() {
	if (session->GetState() != idSession::sessionState_t::LOADING) {
		Warning("Session state is not LOADING in ExecuteMapChange");
		return;
	}

	currentMapName = "test map";

	common->Printf("--------- Execute Map Change ---------\n");
	common->Printf("Map: %s\n", currentMapName.c_str());

	int start = Sys_Milliseconds();

	// close console and remove any prints from the notify lines
	console->Close();

	int sm = Sys_Milliseconds();
	// shut down the existing game if it is running
	UnloadMap();
	int ms = Sys_Milliseconds() - sm;
	common->Printf("%6d msec to unload map\n", ms);

	// Free media from previous level and
	// note which media we are going to need to load
	sm = Sys_Milliseconds();
	renderSystem->BeginLevelLoad();
	ms = Sys_Milliseconds() - sm;
	common->Printf("%6d msec to free assets\n", ms);

	com_engineHz_denominator = 100LL * static_cast<long long>(com_engineHz.GetFloat());

	// let the renderSystem load all the geometry
	if (!renderWorld->InitFromMap("")) {
		common->Error("couldn't load %s"/*, fullMapName.c_str()*/);
	}

	// for the synchronous networking we needed to roll the angles over from
	// level to level, but now we can just clear everything
	usercmdGen->InitForNewMap();

	// load and spawn all other entities ( from a savegame possibly )
	/*if (mapSpawnData.savegameFile) {
		if (!game->InitFromSaveGame(fullMapName, renderWorld, soundWorld, mapSpawnData.savegameFile, mapSpawnData.stringTableFile, mapSpawnData.savegameVersion)) {
			// If the loadgame failed, end the session, which will force us to go back to the main menu
			session->QuitMatchToTitle();
		}
	}
	else
	{*/
	game->InitFromNewMap(currentMapName, renderWorld, Sys_Milliseconds());
	//}

	game->Shell_CreateMenu(true);

	// If the session state is not loading here, something went wrong.
	if (session->GetState() == idSession::sessionState_t::LOADING) {
		// Notify session we are done loading
		session->LoadingFinished();

		while (session->GetState() == idSession::sessionState_t::LOADING) {
			session->UpdateSignInManager();
			Sys_Sleep(10);
		}
	}

	//if (!mapSpawnData.savegameFile)
	{
		// run a single frame to catch any resources that are referenced by events posted in spawn
		/*idUserCmdMgr emptyCommandManager;
		gameReturn_t emptyGameReturn;
		for (int playerIndex = 0; playerIndex < MAX_PLAYERS; ++playerIndex) {
			emptyCommandManager.PutUserCmdForPlayer(playerIndex, usercmd_t());
		}
		if (IsClient()) {
			game->ClientRunFrame(emptyCommandManager, false, emptyGameReturn);
		}
		else {*/
			game->RunFrame(/*emptyCommandManager, emptyGameReturn*/);
		//}
	}

	renderSystem->EndLevelLoad();

	usercmdGen->Clear();

	mapSpawned = true;
	Sys_ClearEvents();

	int	msec = Sys_Milliseconds() - start;
	common->Printf("%6d msec to load %s\n", msec, currentMapName.c_str());
}

void idCommonLocal::UnloadMap()
{
	// end the current map in the game
	if (game) {
		game->MapShutdown();
	}

	mapSpawned = false;
}

void map_f(const idCmdArgs& args) {
	commonLocal.StartNewGame(args.Argv(1), false, -2);
}

/*
==================
idCommonLocal::InitCommands
==================
*/
void idCommonLocal::InitCommands() {
	cmdSystem->AddCommand("map", map_f, CMD_FL_GAME, "loads a map");
}
