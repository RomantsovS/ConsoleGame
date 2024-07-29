#include "idlib/precompiled.h"

#include "Game_local.h"

std::shared_ptr<idRenderWorld> gameRenderWorld; // all drawing is done to this world

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
idGameLocal gameLocal;
idGame* game = &gameLocal;

idCVar game_add_point_delay("game_add_point_delay", "1000", CVAR_SYSTEM | CVAR_INIT, "");
idCVar game_add_point_count("game_add_point_count", "1", CVAR_SYSTEM | CVAR_INIT, "");

idCVar r_info_update_frame_time("r_info_update_frame_time", "100", CVAR_SYSTEM | CVAR_INIT, "");

void AddRandomPoints(const idCmdArgs& args) {
	auto cnt = std::max(game_add_point_count.GetInteger(), 1);

	for (int i = 0; i < cnt; ++i)
		gameLocal.AddRandomPoint();
}

idGameLocal::idGameLocal() {
	Clear();
}

idGameLocal::~idGameLocal() {
}

void idGameLocal::Init() {
	Printf("--------- Initializing Game ----------\n");
	Printf("gamename: %s\n", GAME_VERSION.c_str());
	Printf("gamedate: %s\n", __DATE__);

	info_update_time = r_info_update_frame_time.GetInteger();

	// register game specific decl types
	declManager->RegisterDeclType("model", declType_t::DECL_MODELDEF, idDeclAllocator<idDeclModelDef>);

	// register game specific decl folders
	declManager->RegisterDeclFolder("def", ".def", declType_t::DECL_ENTITYDEF);

	cmdSystem->AddCommand("addrandompoints", AddRandomPoints, CMD_FL_GAME, "addes random point(s)");

	Clear();

	idEvent::Init();
	idClass::Init();

	InitConsoleCommands();

	shellHandler = std::make_shared<idMenuHandler_Shell>();

	//The default config file contains remapped controls that support the XP weapons
	//We want to run this once after the base doom config file has run so we can
	//have the correct xp binds
	cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec default.cfg\n");
	cmdSystem->ExecuteCommandBuffer();

	colors.push_back(colorGreen);
	colors.push_back(colorCyan);
	colors.push_back(colorRed);
	colors.push_back(colorMagenta);
	colors.push_back(colorBrown);
	colors.push_back(colorLightGray);
	colors.push_back(colorLightGreen);
	colors.push_back(colorLightCyan);
	colors.push_back(colorLightRed);
	colors.push_back(colorLightMagenta);
	colors.push_back(colorYellow);
	colors.push_back(colorWhite);

	rand_eng.seed(static_cast<unsigned>(Sys_Time()));

	gamestate = GAMESTATE_NOMAP;

	Printf("game initialized.\n");
	Printf("--------------------------------------\n");
}

/*
===========
idGameLocal::Shutdown

  shut down the entire game
============
*/
void idGameLocal::Shutdown() {
	if (!common) {
		return;
	}

	Printf("------------ Game Shutdown -----------\n");

	Shell_Cleanup();

	MapShutdown();

	idEvent::Shutdown();

	idClass::Shutdown();

	mapFile = nullptr;

	// free the collision map
	collisionModelManager->FreeMap();

	ShutdownConsoleCommands();

	// free memory allocated by class objects
	Clear();

	Printf("--------------------------------------\n");
}

void idGameLocal::InitFromNewMap(const std::string& mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) {
	if (!mapFileName.empty()) {
		MapShutdown();
	}

	Printf("----------- Game Map Init ------------\n");

	gamestate = GAMESTATE_STARTUP;

	gameRenderWorld = renderWorld;

	LoadMap(mapName, randseed);

	MapPopulate();

	SyncPlayersWithLobbyUsers(true);

	gamestate = GAMESTATE_ACTIVE;
}

void idGameLocal::MapShutdown() {
	Printf("--------- Game Map Shutdown ----------\n");

	gamestate = GAMESTATE_SHUTDOWN;

	if (gameRenderWorld) {
		// clear any debug lines, text, and polygons
		gameRenderWorld->DebugClearLines(0);
	}

	MapClear(true);

	clip.Shutdown();
	idClipModel::ClearTraceModelCache();

	collisionModelManager->FreeMap();		// Fixes an issue where when maps were reloaded the materials wouldn't get their surfaceFlags re-set.  Now we free the map collision model forcing materials to be reparsed.

	mapFileName.clear();

	gameRenderWorld = nullptr;

	gamestate = GAMESTATE_NOMAP;

	Printf("--------------------------------------\n");
}

/*
========================
idGameLocal::GetLocalClientNum
========================
*/
int idGameLocal::GetLocalClientNum() const noexcept {
	return 0;
}

/*
===================
idGameLocal::CacheDictionaryMedia

This is called after parsing an EntityDef and for each entity spawnArgs before
merging the entitydef.  It could be done post-merge, but that would
avoid the fast pre-cache check associated with each entityDef
===================
*/
void idGameLocal::CacheDictionaryMedia(gsl::not_null<const idDict*> dict) {
	auto kv = dict->MatchPrefix("model");
	//while (kv) {
	if (kv) {
		if (!kv->second.empty()) {
			//declManager->MediaPrint("Precaching model %s\n", kv->GetValue().c_str());
			// precache model/animations
			if (!declManager->FindType(declType_t::DECL_MODELDEF, kv->second, false)) {
				// precache the render model
				renderModelManager->FindModel(kv->second);
				// precache .cm files only
				collisionModelManager->LoadModel(kv->second);
			}
		}
		//kv = dict->MatchPrefix("model", kv->second);
	}
}

/*
===========
idGameLocal::SpawnPlayer
============
*/
void idGameLocal::SpawnPlayer(int clientNum) {
	idDict args;

	// they can connect
	Printf("SpawnPlayer: %i\n", clientNum);

	args.SetInt("spawn_entnum", clientNum);
	args.Set("name", va("player%d", clientNum + 1));

	// precache the player
	args.Set("classname", gameLocal.world->spawnArgs.GetString("def_player", "player_doommarine"));

	//args.Set("model", "pixel");
	args.Set("color", std::to_string(static_cast<Screen::color_type>(gameLocal.GetRandomColor())));
	args.Set("origin", world->spawnArgs.GetString("player_origin", "10, 10"));

	std::shared_ptr<idEntity> ent;
	if (!SpawnEntityDef(args, &ent) || clientNum >= MAX_GENTITIES || !entities[clientNum]) {
		//Error("Failed to spawn player as '%s'", args.GetString("classname").c_str());
	}

	// make sure it's a compatible class
	if (ent && !ent->IsType(idPlayer::Type)) {
		Error("'%s' spawned the player as a '%s'.  Player spawnclass must be a subclass of idPlayer.", args.GetString("classname").c_str(), ent->GetClassname().c_str());
	}
}

/*
================
idGameLocal::GetLocalPlayer

Nothing in the game tic should EVER make a decision based on what the
local client number is, it shouldn't even be aware that there is a
draw phase even happening.  This just returns client 0, which will
be correct for single player.
================
*/
idPlayer* idGameLocal::GetLocalPlayer() const {
	if (GetLocalClientNum() < 0) {
		return NULL;
	}

	if (!entities.at(GetLocalClientNum()) || !entities.at(GetLocalClientNum())->IsType(idPlayer::Type)) {
		// not fully in game yet
		return NULL;
	}
	return static_cast<idPlayer*>(entities[GetLocalClientNum()].get());
}

/*
================
idGameLocal::RunEntityThink
================
*/
void idGameLocal::RunEntityThink(idEntity& ent, idUserCmdMgr& userCmdMgr) {
	if (ent.entityNumber < MAX_CLIENTS) {
		// Players may run more than one think per frame in MP,
		// if there is a large buffer of usercmds from the network.
		// Players will always run exactly one think in singleplayer.
		RunAllUserCmdsForPlayer(userCmdMgr, ent.entityNumber);
	}
	else {
		// Non-player entities always run one think.
		ent.Think();
	}
}

void idGameLocal::RunFrame(idUserCmdMgr& cmdMgr, gameReturn_t& ret) {
#ifdef _DEBUG
	if (common->IsMultiplayer()) {
		idassert(!common->IsClient());
	}
#endif

	if (!gameRenderWorld) {
		return;
	}

	static auto start_time = Sys_Milliseconds();

	auto player = GetLocalPlayer();

	if (!common->IsMultiplayer() && g_stopTime.GetBool()) {
		// clear any debug lines from a previous frame
		gameRenderWorld->DebugClearLines(time + 1);

		// set the user commands for this frame
		if (player) {
			player->HandleUserCmds(cmdMgr.GetUserCmdForPlayer(GetLocalClientNum()));
			cmdMgr.MakeReadPtrCurrentForPlayer(GetLocalClientNum());
			player->Think();
		}
	}
	else {
		framenum++;
		fast.previousTime = FRAME_TO_MSEC(framenum - 1);
		fast.time = FRAME_TO_MSEC(framenum);
		//fast.previousTime = fast.time;
		//fast.time = FRAME_TO_MSEC(Sys_Milliseconds());
		fast.realClientTime = fast.time;

		if (time == 3000) {
			auto end = Sys_Milliseconds();

			DPrintf("reached 3k game time for %d msec\n", (end - start_time));
		}

		//ComputeSlowScale();

		/*slow.previousTime = slow.time;
		slow.time += idMath::Ftoi((fast.time - fast.previousTime) * slowmoScale);
		slow.realClientTime = slow.time;*/

		SelectTimeGroup(true);

		// make sure the random number counter is used each frame so random events
		// are influenced by the player's actions
		rand_eng.seed(Sys_Milliseconds());

		// clear any debug lines from a previous frame
		gameRenderWorld->DebugClearLines(time + 1);

		static auto lastTimePointSpawn = time;
		static int game_add_point_delay = world->spawnArgs.GetInt("game_add_point_delay");

		if (game_add_point_delay == 0) {
			for (int i = 0; i < world->spawnArgs.GetInt("game_add_point_count", 1000); ++i)
				AddRandomPoint();
			world->spawnArgs.SetInt("game_add_point_delay", -1);
			game_add_point_delay = -1;
		}

		if (game_add_point_delay > 0 && time - lastTimePointSpawn > game_add_point_delay) {
			lastTimePointSpawn = time;

			for (int i = 0; i < world->spawnArgs.GetInt("game_add_point_count", 0); ++i) {
				AddRandomPoint();
			}
		}

		// let entities think
		for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next()) {
			RunEntityThink(*ent, cmdMgr);
		}

		// remove any entities that have stopped thinking
		if (numEntitiesToDeactivate) {
			std::shared_ptr<idEntity> next_ent;
			for (auto ent = activeEntities.Next(); ent != nullptr; ent = next_ent) {
				next_ent = ent->activeNode.Next();
				if (!ent->thinkFlags) {
					ent->activeNode.Remove();
				}
			}
			numEntitiesToDeactivate = 0;
		}

		// service any pending events
		idEvent::ServiceEvents();

		// do multiplayer related stuff
		if (common->IsMultiplayer()) {
			//mpGame.Run();
		}

		BuildReturnValue(ret);
	}

	// show any debug info for this frame
	RunDebugInfo();
}

/*
====================
idGameLocal::BuildReturnValue

Fills out gameReturn_t, called on server and clients.
====================
*/
void idGameLocal::BuildReturnValue(gameReturn_t& ret) {
	ret.sessionCommand.clear();

	// see if a target_sessionCommand has forced a changelevel
	if (!sessionCommand.empty()) {
		ret.sessionCommand = sessionCommand;
		sessionCommand.clear();
	}
}

/*
====================
idGameLocal::RunSinglelUserCmd

Runs a Think or a ClientThink for a player. Will write the client's
position and firecount to the usercmd.
====================
*/
void idGameLocal::RunSingleUserCmd(usercmd_t& cmd, gsl::not_null<idPlayer*> player) {

	player->HandleUserCmds(cmd);

	if (!common->IsMultiplayer() || common->IsServer()) {
		player->Think();

		// Keep track of the client time of the usercmd we just ran. We will send this back to clients
		// in a snapshot so they know when they can stop predicting certain things.
		//usercmdLastClientMilliseconds[player.GetEntityNumber()] = cmd.clientGameMilliseconds;
	}
	else {
		//player->ClientThink(netInterpolationInfo.serverGameMs, netInterpolationInfo.pct, true);
	}
}

/*
====================
idGameLocal::RunAllUserCmdsForPlayer
Runs a Think or ClientThink for each usercmd, but leaves a few cmds in the buffer
so that we have something to process while we wait for more from the network.
====================
*/
void idGameLocal::RunAllUserCmdsForPlayer(idUserCmdMgr& cmdMgr, const int playerNumber) {
	// Run thinks on any players that have queued up usercmds for networking.
	idassert(playerNumber < MAX_PLAYERS);

	if (!entities.at(playerNumber)) {
		return;
	}

	gsl::not_null<idPlayer*> player = dynamic_cast<idPlayer*>(entities.at(playerNumber).get());

	// Only run a single userCmd each game frame for local players, otherwise when
	// we are running < 60fps things like footstep sounds may get started right on top
	// of each other instead of spread out in time.
	if (player->IsLocallyControlled()) {
		if (cmdMgr.HasUserCmdForPlayer(playerNumber)) {
			/*if (net_usercmd_timing_debug.GetBool()) {
				idLib::Printf("[%d]Running local cmd for player %d, %d buffered.\n",
					common->GetGameFrame(), playerNumber, cmdMgr.GetNumUnreadFrames(playerNumber));
			}*/
			RunSingleUserCmd(cmdMgr.GetWritableUserCmdForPlayer(playerNumber), player);
		}
		else {
			RunSingleUserCmd(player->usercmd, player);
		}
		return;
	}

	// Only the server runs remote commands.
	if (common->IsClient()) {
		return;
	}
}

bool idGameLocal::Draw(int clientNum) {
	if (common->IsMultiplayer() && session->GetState() == idSession::sessionState_t::INGAME) {
		//return mpGame.Draw(clientNum);
	}

	//std::shared_ptr<idPlayer> player = std::static_pointer_cast<idPlayer>(entities.at(clientNum));

	//if ((!player) /*|| (player->GetRenderView() == NULL)*/) {
	//	return false;
	//}

	if (!gameRenderWorld)
		return false;

	gameRenderWorld->RenderScene(nullptr);

	return true;
}

/*
================
idGameLocal::RunDebugInfo
================
*/
void idGameLocal::RunDebugInfo() {
	if (time - prev_info_update_time > info_update_time) {
		prev_info_update_time = time;
	}
	else
		return;

	const idPlayer* player = GetLocalPlayer();
	if (!player) {
		return;
	}

	char buf[256];

	size_t num_spawned_entities = 0, num_active_entities = 0;

	for (auto ent = spawnedEntities.Prev(); ent != NULL; ent = ent->spawnNode.Prev())
		++num_spawned_entities;

	for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next())
		++num_active_entities;

	if (g_showEntityInfo.GetBool()) {
		gameRenderWorld->DrawTextToScreen(string_format("ents: %3d, active: %3d", num_spawned_entities,
			num_active_entities), Vector2(), colorYellow, info_update_time + 1);

		if (g_showCollisionTraces.GetBool()) {
			clip.PrintStatistics(info_update_time + 1);
		}

		/*idMat3		axis = player->viewAngles.ToMat3();
		idVec3		up = axis[2] * 5.0f;
		idBounds	viewTextBounds(origin);
		idBounds	viewBounds(origin);

		viewTextBounds.ExpandSelf(128.0f);
		viewBounds.ExpandSelf(512.0f);*/
		for (auto ent = spawnedEntities.Prev(); ent != NULL; ent = ent->spawnNode.Prev()) {
			// don't draw the worldspawn
			/*if (ent == world) {
				continue;
			}*/

			// skip if the entity is very far away
			/*if (!viewBounds.IntersectsBounds(ent->GetPhysics()->GetAbsBounds())) {
				continue;
			}*/

			/*const idBounds &entBounds = ent->GetPhysics()->GetAbsBounds();
			int contents = ent->GetPhysics()->GetContents();
			if (contents & CONTENTS_BODY) {
				gameRenderWorld->DebugBounds(colorCyan, entBounds);
			}
			else if (contents & CONTENTS_TRIGGER) {
				gameRenderWorld->DebugBounds(colorOrange, entBounds);
			}
			else if (contents & CONTENTS_SOLID) {
				gameRenderWorld->DebugBounds(colorGreen, entBounds);
			}
			else {
				if (!entBounds.GetVolume()) {
					gameRenderWorld->DebugBounds(colorMdGrey, entBounds.Expand(8.0f));
				}
				else {
					gameRenderWorld->DebugBounds(colorMdGrey, entBounds);
				}
			}
			if (viewTextBounds.IntersectsBounds(entBounds)) {
				gameRenderWorld->DrawText(ent->name.c_str(), entBounds.GetCenter(), 0.1f, colorWhite, axis, 1);
				gameRenderWorld->DrawText(va("#%d", ent->entityNumber), entBounds.GetCenter() + up, 0.1f, colorWhite, axis, 1);
			}*/
			if (ent->IsActive()) {
				auto phys = ent->GetPhysics();
				auto str = string_format("%10s p[%5.2f %5.2f] v[%6.2f %6.2f] rest %d", ent->GetName().c_str(),
					phys->GetOrigin().x, phys->GetOrigin().y, phys->GetLinearVelocity().x, phys->GetLinearVelocity().y,
					phys->IsAtRest());
				gameRenderWorld->DrawTextToScreen(std::move(str), Vector2(), ent->GetRenderEntity()->color, info_update_time + 1);
			}
		}
	}

	// debug tool to draw bounding boxes around active entities
	/*if (g_showActiveEntities.GetBool()) {
		for (ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next()) {
			idBounds	b = ent->GetPhysics()->GetBounds();
			if (b.GetVolume() <= 0) {
				b[0][0] = b[0][1] = b[0][2] = -8;
				b[1][0] = b[1][1] = b[1][2] = 8;
			}
			if (ent->fl.isDormant) {
				gameRenderWorld->DebugBounds(colorYellow, b, ent->GetPhysics()->GetOrigin());
			}
			else {
				gameRenderWorld->DebugBounds(colorGreen, b, ent->GetPhysics()->GetOrigin());
			}
		}
	}

	if (g_showTargets.GetBool()) {
		ShowTargets();
	}

	if (g_showTriggers.GetBool()) {
		idTrigger::DrawDebugInfo();
	}

	if (ai_showCombatNodes.GetBool()) {
		idCombatNode::DrawDebugInfo();
	}

	if (ai_showPaths.GetBool()) {
		idPathCorner::DrawDebugInfo();
	}

	if (g_editEntityMode.GetBool()) {
		editEntities->DisplayEntities();
	}

	if (g_showCollisionWorld.GetBool()) {
		collisionModelManager->DrawModel(0, vec3_origin, mat3_identity, origin, 128.0f);
	}*/

	if (g_showCollisionModels.GetBool()) {
		clip.DrawClipModels(player->GetPhysics()->GetOrigin(), g_maxShowDistance.GetFloat(), nullptr);
	}

	/*if (g_showPVS.GetInteger()) {
		pvs.DrawPVS(origin, (g_showPVS.GetInteger() == 2) ? PVS_ALL_PORTALS_OPEN : PVS_NORMAL);
	}

	if (aas_test.GetInteger() >= 0) {
		idAAS *aas = GetAAS(aas_test.GetInteger());
		if (aas) {
			aas->Test(origin);
			if (ai_testPredictPath.GetBool()) {
				idVec3 velocity;
				predictedPath_t path;

				velocity.x = cos(DEG2RAD(player->viewAngles.yaw)) * 100.0f;
				velocity.y = sin(DEG2RAD(player->viewAngles.yaw)) * 100.0f;
				velocity.z = 0.0f;
				idAI::PredictPath(player, aas, origin, velocity, 1000, 100, SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA, path);
			}
		}
	}

	if (ai_showObstacleAvoidance.GetInteger() == 2) {
		idAAS *aas = GetAAS(0);
		if (aas) {
			idVec3 seekPos;
			obstaclePath_t path;

			seekPos = player->GetPhysics()->GetOrigin() + player->viewAxis[0] * 200.0f;
			idAI::FindPathAroundObstacles(player->GetPhysics(), aas, NULL, player->GetPhysics()->GetOrigin(), seekPos, path);
		}
	}

	// collision map debug output
	collisionModelManager->DebugOutput(player->GetEyePosition());*/

	if (g_showCollisionTraces.GetBool()) {
		clip.DrawClipSectors();
	}
}

void idGameLocal::PrintSpawnedEntities()
{
	for (auto ent = spawnedEntities.Next(); ent != nullptr; ent = ent->spawnNode.Next()) {
		Printf("ent %s pos [%6.3f %6.3f] vel [%6.3f %6.3f] rest %d", ent->GetName().c_str(),
			ent->GetPhysics()->GetOrigin().x, ent->GetPhysics()->GetOrigin().y,
			ent->GetPhysics()->GetLinearVelocity().x, ent->GetPhysics()->GetLinearVelocity().y,
			ent->GetPhysics()->IsAtRest());
	}
}

/*
============
idGameLocal::Printf
============
*/
void idGameLocal::Printf(const char* fmt, ...) const {
	va_list argptr;
	char text[max_string_chars];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Printf("%s", text);
}

/*
============
idGameLocal::DPrintf
============
*/
void idGameLocal::DPrintf(const char* fmt, ...) const {
	va_list argptr;
	char text[max_string_chars];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Printf("%s", text);
}

/*
============
idGameLocal::Warning
============
*/
void idGameLocal::Warning(const char* fmt, ...) const {
	va_list argptr;
	char text[max_string_chars];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Warning("%s", text);
}

/*
============
idGameLocal::DWarning
============
*/
void idGameLocal::DWarning(const char* fmt, ...) const {
	va_list		argptr;
	char text[max_string_chars];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->DWarning("%s", text);
}

/*
============
idGameLocal::Error
============
*/
void idGameLocal::Error(const char* fmt, ...) const {
	va_list argptr;
	char text[max_string_chars];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Error("%s", text);
}

void idGameLocal::LoadMap(const std::string& mapName, int randseed) {
	bool sameMap = mapFile && mapFileName == mapName;

	if (!sameMap || (mapFile && mapFile->NeedsReload())) {
		// load the .map file
		if (mapFile) {
			mapFile = nullptr;
		}
		mapFile = std::make_shared<idMapFile>();
		if (!mapFile->Parse(mapName + ".map")) {
			mapFile = nullptr;
			Error("Couldn't load %s", mapName.c_str());
		}
	}
	mapFileName = mapFile->GetName();

	getGameSize(mapFile.get());

	// load the collision map
	collisionModelManager->LoadMap(mapFile.get());
	//collisionModelManager->Preload(mapName);

	entities.clear();
	entities.resize(MAX_GENTITIES);
	std::fill(spawnIds.begin(), spawnIds.end(), -1);
	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	num_entities = MAX_CLIENTS;
	firstFreeEntityIndex[0] = MAX_CLIENTS;

	world = nullptr;

	previousTime = 0;
	time = 0;
	prev_info_update_time = 0;
	framenum = 0;
	sessionCommand.clear();

	spawnArgs.Clear();

	clip.Init();

	if (!sameMap) {
		mapFile->RemovePrimitiveData();
	}
}

void idGameLocal::getGameSize(const idMapFile* mapFile) {
	if (mapFile->GetNumEntities() == 0) {
		Error("Map file has zero entities");
	}
	const std::shared_ptr<idMapEntity> mapEnt = mapFile->GetEntity(0);
	if (mapEnt->GetNumPrimitives() != 1) {
		Error("Map file must have only one primitive");
	}
	std::shared_ptr<idMapPrimitive> mapPrim = mapEnt->GetPrimitive(0);
	if (mapPrim->GetType() != idMapPrimitive::TYPE_BRUSH) {
		Error("Map file first entity must be brush");
		
	}
	auto mapBrush = dynamic_cast<idMapBrush*>(mapPrim.get());
	if (mapBrush->GetNumSides() != 1) {
		Error("Map brush must have only one side");
	}
	std::shared_ptr<idMapBrushSide> mapSide = mapBrush->GetSide(0);
	if (mapSide->GetPoints().size() != 2) {
		Error("Map brush must have two points");
	}
	width = mapSide->GetPoints()[1].x - mapSide->GetPoints()[0].x;
	width = mapSide->GetPoints()[1].y - mapSide->GetPoints()[0].y;
}

void idGameLocal::Clear() {
	entities.clear();
	entities.resize(MAX_GENTITIES);
	spawnIds.resize(MAX_GENTITIES);
	std::fill(spawnIds.begin(), spawnIds.end(), -1);
	firstFreeEntityIndex[0] = 0;
	firstFreeEntityIndex[1] = ENTITYNUM_FIRST_NON_REPLICATED;
	num_entities = 0;
	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;
	world = nullptr;

	clip.Shutdown();
	sessionCommand.clear();

	framenum = 0;
	previousTime = 0;
	prev_info_update_time = 0;
	time = 0;
	mapFileName.clear();
	mapFile = nullptr;
	spawnArgs.Clear();
	gamestate = GAMESTATE_UNINITIALIZED;

	shellHandler = nullptr;

	ResetSlowTimeVars();

	colors.clear();
}

/*
==============
idGameLocal::GameState

Used to allow entities to know if they're being spawned during the initial spawn.
==============
*/
gameState_t	idGameLocal::GameState() const noexcept {
	return gamestate;
}

/*
========================
idGameLocal::InhibitControls
========================
*/
bool idGameLocal::InhibitControls() {
	return Shell_IsActive();
}

/*
========================
idGameLocal::Shell_Init
========================
*/
void idGameLocal::Shell_Init(const std::string& filename) {
	if (shellHandler) {
		shellHandler->Initialize(filename);
	}
}

/*
========================
idGameLocal::Shell_Init
========================
*/
void idGameLocal::Shell_Cleanup() noexcept {
	if (shellHandler) {
		shellHandler = nullptr;
	}
}

/*
========================
idGameLocal::Shell_CreateMenu
========================
*/
void idGameLocal::Shell_CreateMenu(bool inGame) {
	Shell_ResetMenu();

	if (shellHandler) {
		if (!inGame) {
			shellHandler->SetInGame(false);
			Shell_Init("shell");
		}
		else {
			shellHandler->SetInGame(true);
			Shell_Init("pause");

		}
	}
}

/*
========================
idGameLocal::Shell_Show
========================
*/
void idGameLocal::Shell_Show(bool show) noexcept {
	if (shellHandler) {
		shellHandler->ActivateMenu(show);
	}
}

/*
========================
idGameLocal::Shell_IsActive
========================
*/
bool idGameLocal::Shell_IsActive() const noexcept {
	if (shellHandler) {
		return shellHandler->IsActive();
	}
	return false;
}

/*
========================
idGameLocal::Shell_HandleGuiEvent
========================
*/
bool idGameLocal::Shell_HandleGuiEvent(const sysEvent_t* sev) {
	if (shellHandler != NULL) {
		return shellHandler->HandleGuiEvent(sev);
	}
	return false;
}

/*
========================
idGameLocal::Shell_Render
========================
*/
void idGameLocal::Shell_Render() {
	if (shellHandler) {
		shellHandler->Update();
	}
}

/*
========================
idGameLocal::Shell_ResetMenu
========================
*/
void idGameLocal::Shell_ResetMenu() {
	if (shellHandler) {
		shellHandler = nullptr;

		shellHandler = std::make_shared<idMenuHandler_Shell>();
	}
}

/*
=================
idGameLocal::Shell_SyncWithSession
=================
*/
void idGameLocal::Shell_SyncWithSession() {
	if (!shellHandler) {
		return;
	}
	switch (session->GetState()) {
	case idSession::sessionState_t::PRESS_START:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_PRESS_START);
		break;
	case idSession::sessionState_t::INGAME:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_PAUSED);
		break;
	case idSession::sessionState_t::IDLE:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_IDLE);
		break;
	case idSession::sessionState_t::PARTY_LOBBY:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_PARTY_LOBBY);
		break;
	case idSession::sessionState_t::GAME_LOBBY:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_GAME_LOBBY);
		break;
	case idSession::sessionState_t::SEARCHING:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_SEARCHING);
		break;
	case idSession::sessionState_t::LOADING:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_LOADING);
		break;
	case idSession::sessionState_t::CONNECTING:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_CONNECTING);
		break;
	case idSession::sessionState_t::BUSY:
		shellHandler->SetShellState(shellState_t::SHELL_STATE_BUSY);
		break;
	}
}

void idGameLocal::SpawnMapEntities() {
	int			i;
	int			num;
	int			numEntities;
	idDict		args;

	Printf("Spawning entities\n");

	if (!mapFile) {
		Printf("No mapfile present\n");
		return;
	}

	numEntities = mapFile->GetNumEntities();
	if (numEntities == 0) {
		Error("...no entities");
	}

	// the worldspawn is a special that performs any global setup
	// needed by a level
	std::shared_ptr<idMapEntity> mapEnt = mapFile->GetEntity(0);
	args = mapEnt->epairs;
	args.SetInt("spawn_entnum", ENTITYNUM_WORLD);
	if (!SpawnEntityDef(args) || !entities[ENTITYNUM_WORLD] || !entities[ENTITYNUM_WORLD]->IsType(idWorldspawn::Type)) {
		Error("Problem spawning world entity");
	}

	tr.screen->setBackGroundPixel({ '\xDB',
	static_cast<Screen::color_type>(args.GetInt("r_background_color")) });

	num = 1;

	for (i = 1; i < numEntities; i++) {
		mapEnt = mapFile->GetEntity(i);
		args = mapEnt->epairs;

		// precache any media specified in the map entity
		CacheDictionaryMedia(&args);

		SpawnEntityDef(args);
		num++;
	}

	Printf("...%i entities spawned\n\n", num);
}

void idGameLocal::MapPopulate() {
	// parse the key/value pairs and spawn entities
	SpawnMapEntities();

	// execute pending events before the very first game frame
	// this makes sure the map script main() function is called
	// before the physics are run so entities can bind correctly
	Printf("==== Processing events ====\n");
	idEvent::ServiceEvents();
}

void idGameLocal::MapClear(bool clearClients) noexcept {
	for (size_t i = (clearClients ? 0 : MAX_CLIENTS); i < MAX_GENTITIES; i++) {
		auto ent = entities[i];

		if (ent) {
			ent->Remove();
		}
		spawnIds[i] = -1;
	}
}

void idGameLocal::AddRandomPoint() {
	idDict args;

	const size_t ent_type = GetRandomValue(3, 10);

	float searching_radius = 0.0f;
	float start_pos = 0.0f;
	std::string classname;

	if (ent_type == 0) {
		classname = "mushroom_static";
	}
	/*else if (ent_type < 3) {
		classname = "chain";

		const size_t links = GetRandomValue(3, 5);
		args.Set("links", std::to_string(links));
		searching_radius = static_cast<float>(links);
	}*/
	else if (ent_type < 8) {
		classname = "mushroom";
	}
	else {
		classname = "turtle";
	}

	args.Set("classname", classname);

	const std::shared_ptr<idDeclEntityDef> def = FindEntityDef(classname, false);

	if (!def) {
		Warning("Unknown classname '%s'.", classname.c_str());
		return;
	}

	auto size_x = def->dict.GetVector("size").x;
	auto size_y = def->dict.GetVector("size").y;
	auto size_max = std::max(size_x, size_y);
	searching_radius = std::max(searching_radius, size_max) / 2.0f;
	start_pos += searching_radius;

	Vector2 origin(GetRandomValue(start_pos, GetWidth() - size_max), GetRandomValue(start_pos, GetHeight() - size_max));
	//Vector2 origin = { 136.0f, 156.0f };
	const Vector2 axis(0, 0);

	std::vector<std::shared_ptr<idEntity>> ent_vec(1);

	int num_attempts = 0;
	int finded_ents = 0;
	while ((finded_ents = EntitiesWithinRadius(origin, searching_radius, ent_vec, ent_vec.size())) != 0) {
		if (num_attempts++ > 1000) {
			Warning("couldn't spawn random point at %5.2f %5.2f, finded %d with radius %5.2f", origin.x, origin.y, finded_ents, searching_radius);
			return;
		}

		origin = Vector2(GetRandomValue(start_pos, GetWidth() - size_max), GetRandomValue(start_pos, GetHeight() - size_max));
	}

	args.Set("origin", origin.ToString());
	args.Set("axis", axis.ToString());
	args.Set("size", va("%s %s", std::to_string(size_x).c_str(), std::to_string(size_y).c_str()));
	args.Set("color", std::to_string(static_cast<Screen::color_type>(gameLocal.GetRandomColor())));

	int speed = def->dict.GetInt("speed");
	auto rand = gameLocal.GetRandomValue(0.0f, 1.0f);
	if (gameLocal.GetRandomValue(0.0f, 1.0f) > 0)
		speed = -speed;

	args.Set("linearVelocity", (rand > 0) ? Vector2(speed, 0).ToString() : Vector2(0, speed).ToString());

	std::shared_ptr<idEntity> ent;
	if (!gameLocal.SpawnEntityDef(args, &ent)) {
		Warning("Failed to spawn random point as '%s'", args.GetString("classname").c_str());
	}
	else {
		//ent->PostEventMS(&EV_Remove, 1000);
	}


	/*args.Set("classname", "idSimpleObject");
	args.Set("spawnclass", "idSimpleObject");
	args.Set("origin", Vector2(4.0f, 5.0f).ToString());
	args.Set("linearVelocity", (Vector2(0.0f, 0.0f).ToString()));
	args.Set("color", std::to_string(colorLightRed));
	gameLocal.SpawnEntityDef(args, &ent);*/
}

Screen::color_type idGameLocal::GetRandomColor() {
	std::uniform_int_distribution<Screen::color_type> u_c(0, colors.size() - 1);

	int col(u_c(rand_eng));

	return colors[col];
}

/*
================
idGameLocal::SpawnEntityType
================
*/
std::shared_ptr<idEntity> idGameLocal::SpawnEntityType(const idTypeInfo& classdef, const idDict* args, bool bIsClientReadSnapshot) {
	std::shared_ptr<idClass> obj;

	//#if _DEBUG
	//	if (common->IsClient()) {
	//		idassert(bIsClientReadSnapshot);
	//	}
	//#endif

	if (!classdef.IsType(idEntity::Type)) {
		Error("Attempted to spawn non-entity class '%s'", classdef.classname.c_str());
	}

	try {
		if (args) {
			spawnArgs = *args;
		}
		else {
			spawnArgs.Clear();
		}
		obj = classdef.CreateInstance();
		obj->CallSpawn();
	}
	catch (std::exception&) {
		obj = nullptr;
	}
	spawnArgs.Clear();

	return std::static_pointer_cast<idEntity>(obj);
}

bool idGameLocal::SpawnEntityDef(const idDict& args, std::shared_ptr<idEntity>* ent) {
	std::string classname, spawn, error, name;
	idTypeInfo* cls;
	std::shared_ptr<idClass> obj;

	if (ent) {
		*ent = nullptr;
	}

	spawnArgs = args;

	if (spawnArgs.GetString("name", "", &name)) {
		sprintf(error, " on '%s'", name.c_str());
	}

	spawnArgs.GetString("classname", "", &classname);

	const std::shared_ptr<idDeclEntityDef> def = FindEntityDef(classname, false);

	if (!def) {
		Warning("Unknown classname '%s'%s.", classname.c_str(), error.c_str());
		return false;
	}

	spawnArgs.SetDefaults(&def->dict);

	// check if we should spawn a class object
	spawnArgs.GetString("spawnclass", "", &spawn);

	if (!spawn.empty()) {

		cls = idClass::GetClass(spawn);
		if (!cls) {
			Warning("Could not spawn '%s'. Class '%s' not found%s.", classname.c_str(), spawn.c_str(), error.c_str());
			return false;
		}

		obj = cls->CreateInstance();
		if (!obj) {
			Warning("Could not spawn '%s'. Instance could not be created%s.", classname, error.c_str());
			return false;
		}

		obj->CallSpawn();

		if (ent && obj->IsType(idEntity::Type)) {
			*ent = std::dynamic_pointer_cast<idEntity>(obj);
		}

		return true;
	}

	return false;
}

/*
================
idGameLocal::FindEntityDef
================
*/
const std::shared_ptr<idDeclEntityDef> idGameLocal::FindEntityDef(const std::string& name, bool makeDefault) const {
	const std::shared_ptr<idDecl> decl = declManager->FindType(declType_t::DECL_ENTITYDEF, name, makeDefault);
	return std::dynamic_pointer_cast<idDeclEntityDef>(decl);
}

/*
================
idGameLocal::FindEntityDefDict
================
*/
const idDict* idGameLocal::FindEntityDefDict(const std::string& name, bool makeDefault) const {
	const std::shared_ptr<idDeclEntityDef> decl = FindEntityDef(name, makeDefault);
	return decl ? &decl->dict : nullptr;
}

/*
==================
idGameLocal::CheatsOk
==================
*/
bool idGameLocal::CheatsOk(bool requirePlayer) {
	if (developer.GetBool()) {
		return true;
	}

	idPlayer* player = GetLocalPlayer();
	if (!requirePlayer || (player && (player->health > 0))) {
		return true;
	}

	Printf("You must be alive to use this command.\n");

	return false;
}

void idGameLocal::RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict& spawnArgsToCopy) {
	int spawn_entnum;

	if (spawnCount >= (1 << (32 - GENTITYNUM_BITS))) {
		Error("idGameLocal::RegisterEntity: spawn count overflow");
	}

	if (!spawnArgsToCopy.GetInt("spawn_entnum", "0", spawn_entnum)) {
		const int freeListType = 0;
		const int maxEntityNum = ENTITYNUM_MAX_NORMAL;
		int freeIndex = firstFreeEntityIndex[freeListType];

		while (entities[freeIndex] && freeIndex < maxEntityNum) {
			freeIndex++;
		}
		if (freeIndex >= maxEntityNum) {
			Error("entities overflow %d %d", freeIndex, maxEntityNum);
		}
		spawn_entnum = freeIndex++;

		firstFreeEntityIndex[freeListType] = freeIndex;
	}

	entities[spawn_entnum] = ent;
	spawnIds[spawn_entnum] = (forceSpawnId >= 0) ? forceSpawnId : spawnCount++;
	ent->entityNumber = spawn_entnum;
	ent->spawnNode.AddToEnd(spawnedEntities);

	// Make a copy because TransferKeyValues clears the input parameter.
	idDict copiedArgs = spawnArgsToCopy;
	//ent->spawnArgs.TransferKeyValues(copiedArgs);

	ent->spawnArgs = copiedArgs;

	if (spawn_entnum >= num_entities) {
		num_entities++;
	}
}

void idGameLocal::UnregisterEntity(std::shared_ptr<idEntity> ent) noexcept {
	if ((ent->entityNumber != ENTITYNUM_NONE) && (entities[ent->entityNumber] == ent)) {
		ent->spawnNode.Remove();

		if (g_debugSpawn.GetBool())
			DPrintf("Unregister ent %d %s %s\n", ent->entityNumber, ent->GetClassname().c_str(), ent->name.c_str());

		entities[ent->entityNumber] = nullptr;
		spawnIds[ent->entityNumber] = -1;

		int freeListType = (ent->entityNumber >= ENTITYNUM_FIRST_NON_REPLICATED) ? 1 : 0;

		if (ent->entityNumber >= MAX_CLIENTS && ent->entityNumber < firstFreeEntityIndex[freeListType]) {
			firstFreeEntityIndex[freeListType] = ent->entityNumber;
		}

		ent->entityNumber = ENTITYNUM_NONE;
	}
}

/*
================
idGameLocal::EntitiesWithinRadius
================
*/
int idGameLocal::EntitiesWithinRadius(const Vector2 org, float radius, std::vector<std::shared_ptr<idEntity>>& entityList, int maxCount) const {
	std::shared_ptr<idEntity> ent;
	idBounds bo(org);
	bo.AddPoint(org + vec2_point_size);

	int entCount = 0;

	bo.ExpandSelf(radius);
	for (ent = spawnedEntities.Next(); ent != NULL; ent = ent->spawnNode.Next()) {
		if (ent->GetPhysics()->GetAbsBounds().IntersectsBounds(bo)) {
			entityList[entCount++] = ent;

			if (entCount >= maxCount)
				break;
		}
	}

	return entCount;
}

/*
===========
idGameLocal::SelectInitialSpawnPoint
spectators are spawned randomly anywhere
in-game clients are spawned based on distance to active players (randomized on the first half)
upon map restart, initial spawns are used (randomized ordered list of spawns flagged "initial")
  if there are more players than initial spots, overflow to regular spawning
============
*/
Vector2 idGameLocal::SelectInitialSpawnPoint(idPlayer* player) {
	Vector2 origin = player->spawnArgs.GetVector("origin", "0, 0");

	/*std::vector<std::shared_ptr<idEntity>> ent_vec(1);

	int num_attempts = 0;
	float searching_radius = 0.0f;
	int finded_ents = 0;
	while ((finded_ents = EntitiesWithinRadius(origin, searching_radius, ent_vec, ent_vec.size())) != 0)
	{
		if (num_attempts++ > 100)
		{
			Error("couldn't spawn random point at %5.2f %5.2f, finded %d with radius %f", origin.x, origin.y, finded_ents, searching_radius);
		}

		origin = Vector2(GetRandomValue(0.0f, GetHeight() - 1.0f), GetRandomValue(0.0f, GetWidth() - 1.0f));
	}*/

	return origin;
}

/*
===========
idGameLocal::SelectTimeGroup
============
*/
void idGameLocal::SelectTimeGroup(int timeGroup) noexcept {
	if (timeGroup) {
		fast.Get(time, previousTime/*, realClientTime*/);
	}
	else {
		slow.Get(time, previousTime/*, realClientTime*/);
	}

	selectedGroup = timeGroup;
}

/*
===========
idGameLocal::GetTimeGroupTime
============
*/
int idGameLocal::GetTimeGroupTime(int timeGroup) noexcept {
	if (timeGroup) {
		return fast.time;
	}
	else {
		return slow.time;
	}
}

/*
===========
idGameLocal::ResetSlowTimeVars
============
*/
void idGameLocal::ResetSlowTimeVars() noexcept {
	//slowmoScale = 1.0f;
	//slowmoState = SLOWMO_STATE_OFF;

	fast.previousTime = 0;
	fast.time = 0;

	slow.previousTime = 0;
	slow.time = 0;
}