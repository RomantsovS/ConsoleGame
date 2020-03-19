#include "Common_local.h"
#include "../d3xp/Game.h"
#include "../renderer/RenderSystem.h"

/*
===============
idCommonLocal::ExecuteMapChange

Performs the initialization of a game based on session match parameters, used for both single
player and multiplayer, but not for renderDemos, which don't create a game at all.
Exits with mapSpawned = true
===============
*/
void idCommonLocal::ExecuteMapChange()
{
	// Free media from previous level and
	// note which media we are going to need to load
	renderSystem->BeginLevelLoad();

	// let the renderSystem load all the geometry
	if (!renderWorld->InitFromMap(""))
	{
		//common->Error("couldn't load %s", fullMapName.c_str());
	}

	// load and spawn all other entities ( from a savegame possibly )
	/*if (mapSpawnData.savegameFile) {
		if (!game->InitFromSaveGame(fullMapName, renderWorld, soundWorld, mapSpawnData.savegameFile, mapSpawnData.stringTableFile, mapSpawnData.savegameVersion)) {
			// If the loadgame failed, end the session, which will force us to go back to the main menu
			session->QuitMatchToTitle();
		}
	}
	else
	{*/
		game->InitFromNewMap("test", renderWorld, Sys_Milliseconds());
	//}

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

	mapSpawned = true;
}