#include <exception>

#include "Game_local.h"
#include "../d3xp/gamesys/Class.h"
#include "../renderer/RenderWorld_local.h"
#include "../framework/Common.h"
#include "../idlib/Lib.h"
#include "../idlib/Str.h"
#include "../framework/CmdSystem.h"
#include "Player.h"
#include "../framework/UsercmdGen.h"
#include "gamesys/SysCvar.h"

std::shared_ptr<idRenderWorld> gameRenderWorld; // all drawing is done to this world

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
idGameLocal gameLocal;
idGame *game = &gameLocal;

const size_t frame_time_min = 10;

idCVar game_width("game_width", "80", CVAR_SYSTEM | CVAR_INIT, "");
idCVar game_height("game_height", "50", CVAR_SYSTEM | CVAR_INIT, "");

idCVar game_add_point_delay("game_add_point_delay", "1000", CVAR_SYSTEM | CVAR_INIT, "");
idCVar game_add_point_count("game_add_point_count", "1", CVAR_SYSTEM | CVAR_INIT, "");

void AddRandomPoints(const idCmdArgs& args) {
	for (int i = 0; i < 100; ++i)
		gameLocal.AddRandomPoint();
}

idGameLocal::idGameLocal()
{
	Clear();
}

void idGameLocal::Init()
{
	Printf("--------- Initializing Game ----------\n");
	Printf("gamename: %s\n", GAME_VERSION.c_str());
	Printf("gamedate: %s\n", __DATE__);

	cmdSystem->AddCommand("addrandompoints", AddRandomPoints, CMD_FL_GAME, "addes random point(s)");

	Clear();

	clip = std::make_shared<idClip>();

	idEvent::Init();
	idClass::Init();

	InitConsoleCommands();

	//The default config file contains remapped controls that support the XP weapons
	//We want to run this once after the base doom config file has run so we can
	//have the correct xp binds
	cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "exec default.cfg\n");
	cmdSystem->ExecuteCommandBuffer();

	height = game_height.GetInteger();
	width = game_width.GetInteger();

	//colors.push_back(Screen::Green);
	colors.push_back(Screen::Cyan);
	colors.push_back(Screen::Red);
	colors.push_back(Screen::Magenta);
	//colors.push_back(Screen::Brown);
	colors.push_back(Screen::LightGray);
	colors.push_back(Screen::LightGreen);
	colors.push_back(Screen::LightCyan);
	colors.push_back(Screen::LightRed);
	colors.push_back(Screen::LightMagenta);
	colors.push_back(Screen::Yellow);
	colors.push_back(Screen::White);

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
void idGameLocal::Shutdown()
{
	if (!common)
	{
		return;
	}

	Printf("------------ Game Shutdown -----------\n");

	MapShutdown();

	idEvent::Shutdown();

	idClass::Shutdown();

	// free the collision map
	collisionModelManager->FreeMap();

	ShutdownConsoleCommands();

	// free memory allocated by class objects
	Clear();

	Printf("--------------------------------------\n");
}

void idGameLocal::InitFromNewMap(const std::string &mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed)
{
	if (!mapFileName.empty())
	{
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

void idGameLocal::MapShutdown()
{
	Printf("--------- Game Map Shutdown ----------\n");

	gamestate = GAMESTATE_SHUTDOWN;

	if (gameRenderWorld) {
		// clear any debug lines, text, and polygons
		gameRenderWorld->DebugClearLines(0);
	}

	MapClear(true);

	if(clip)
		clip->Shutdown();
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
int idGameLocal::GetLocalClientNum() const {
	return 0;
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
	args.Set("classname", "idPlayer");
	args.Set("spawnclass", "idPlayer");

	args.Set("model", "pixel");
	args.Set("color", std::to_string(gameLocal.GetRandomColor()));
	args.Set("origin", Vector2(15.0f, 0.16f).ToString());
	args.Set("linearVelocity", Vector2(0.0f, 10.0f).ToString());

	std::shared_ptr<idEntity> ent;
	if (!SpawnEntityDef(args, &ent) || clientNum >= MAX_GENTITIES || !entities[clientNum]) {
		Error("Failed to spawn player as '%s'", args.GetString("classname").c_str());
	}

	// make sure it's a compatible class
	if (!ent->IsType(idPlayer::Type)) {
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
std::shared_ptr<idPlayer> idGameLocal::GetLocalPlayer() const {
	if (GetLocalClientNum() < 0) {
		return NULL;
	}

	if (!entities[GetLocalClientNum()] || !entities[GetLocalClientNum()]->IsType(idPlayer::Type)) {
		// not fully in game yet
		return NULL;
	}
	return std::static_pointer_cast<idPlayer>(entities[GetLocalClientNum()]);
}

/*
================
idGameLocal::RunEntityThink
================
*/
void idGameLocal::RunEntityThink(idEntity& ent/*, idUserCmdMgr& userCmdMgr*/) {
	if (ent.entityNumber < MAX_CLIENTS) {
		// Players may run more than one think per frame in MP,
		// if there is a large buffer of usercmds from the network.
		// Players will always run exactly one think in singleplayer.
		RunAllUserCmdsForPlayer(/*userCmdMgr,*/ ent.entityNumber);
	}
	else {
		// Non-player entities always run one think.
		ent.Think();
	}
}

void idGameLocal::RunFrame()
{
	if (!gameRenderWorld)
	{
		return;
	}

	auto player = GetLocalPlayer();

	framenum++;
	fast.previousTime = FRAME_TO_MSEC(framenum - 1);
	fast.time = FRAME_TO_MSEC(framenum);
	//fast.previousTime = fast.time;
	//fast.time = FRAME_TO_MSEC(Sys_Milliseconds());
	fast.realClientTime = fast.time;

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
	if (time - lastTimePointSpawn > game_add_point_delay.GetInteger()) {
		lastTimePointSpawn = time;
		
		for (int i = 0; i < game_add_point_count.GetInteger(); ++i) {
			AddRandomPoint();
		}
	}

	// let entities think
	for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next())
	{
		RunEntityThink(*ent);
	}

	// remove any entities that have stopped thinking
	if (numEntitiesToDeactivate) {
		std::shared_ptr<idEntity> next_ent;
		for (auto ent = activeEntities.Next(); ent != nullptr; ent = next_ent)
		{
			next_ent = ent->activeNode.Next();
			if (!ent->thinkFlags) {
				ent->activeNode.Remove();
			}
		}
		numEntitiesToDeactivate = 0;
	}

	// service any pending events
	idEvent::ServiceEvents();

	// show any debug info for this frame
	RunDebugInfoScreen();
	RunDebugInfo();
}

/*
====================
idGameLocal::RunSinglelUserCmd

Runs a Think or a ClientThink for a player. Will write the client's
position and firecount to the usercmd.
====================
*/
void idGameLocal::RunSingleUserCmd(usercmd_t& cmd, idPlayer& player) {
	player.HandleUserCmds(cmd);

	player.Think();
}

/*
====================
idGameLocal::RunAllUserCmdsForPlayer
Runs a Think or ClientThink for each usercmd, but leaves a few cmds in the buffer
so that we have something to process while we wait for more from the network.
====================
*/
void idGameLocal::RunAllUserCmdsForPlayer(/*idUserCmdMgr& cmdMgr,*/ const int playerNumber) {
	// Run thinks on any players that have queued up usercmds for networking.
	//assert(playerNumber < MAX_PLAYERS);

	if (!entities[playerNumber]) {
		return;
	}

	idPlayer& player = static_cast<idPlayer&>(*entities[playerNumber]);

	// Only run a single userCmd each game frame for local players, otherwise when
	// we are running < 60fps things like footstep sounds may get started right on top
	// of each other instead of spread out in time.
	if (player.IsLocallyControlled()) {
		auto cmd = usercmdGen->GetCurrentUsercmd();
		RunSingleUserCmd(cmd, player);
		return;
	}
}

bool idGameLocal::Draw(int clientNum)
{
	RB_RenderDebugToolsBefore();

	gameRenderWorld->RenderScene(nullptr);

	return true;
}

/*
================
idGameLocal::RunDebugInfo
================
*/
void idGameLocal::RunDebugInfo() {
	if (!tr.update_info)
		return;

	/*idPlayer *player;

	player = GetLocalPlayer();
	if (!player) {
		return;
	}

	const idVec3 &origin = player->GetPhysics()->GetOrigin();*/

	char buf[256];
	
	size_t num_spawned_entities = 0, num_active_entities = 0;

	for (auto ent = spawnedEntities.Prev(); ent != NULL; ent = ent->spawnNode.Prev())
		++num_spawned_entities;

	for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next())
		++num_active_entities;

	sprintf_s(buf, "num ents %3d, active ents %3d", num_spawned_entities, num_active_entities);

	gameRenderWorld->DrawText(buf, Vector2(), Screen::ConsoleColor::Yellow, 0);

	clip->PrintStatistics();

	if (/*g_showEntityInfo.GetBool()*/true) {
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
			if (true || ent->IsActive())
			{
				static char buf[256];
				sprintf_s(buf, "ent %30s pos [%6.2f %6.2f] vel [%6.3f %6.3f] rest %d", ent->GetName().c_str(),
					ent->GetPhysics()->GetOrigin().x, ent->GetPhysics()->GetOrigin().y,
					ent->GetPhysics()->GetLinearVelocity().x, ent->GetPhysics()->GetLinearVelocity().y,
					ent->GetPhysics()->IsAtRest());

				gameRenderWorld->DrawText(buf, Vector2(), ent->GetRenderEntity()->color, 0);
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

	/*if (g_showCollisionModels.GetBool()) {
		clip.DrawClipModels(player->GetEyePosition(), g_maxShowDistance.GetFloat(), pm_thirdPerson.GetBool() ? NULL : player);
	}*/

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
}

/*
================
idGameLocal::RunDebugInfoScreen
================
*/
void idGameLocal::RunDebugInfoScreen() {
	if (!tr.update_frame)
		return;

	if (g_showCollisionTraces.GetBool()) {
		clip->DrawClipSectors();
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
void idGameLocal::Printf(const char* fmt, ...) const
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

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
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

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
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

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
	char		text[MAX_STRING_CHARS];

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
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Error("%s", text);
}

void idGameLocal::LoadMap(const std::string mapName, int randseed)
{
	mapFileName = mapName;

	// load the collision map
	collisionModelManager->LoadMap(/*mapFile*/);
	//collisionModelManager->Preload(mapName);

	entities.clear();
	entities.resize(MAX_GENTITIES);
	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	num_entities = MAX_CLIENTS;
	firstFreeEntityIndex[0] = MAX_CLIENTS;

	previousTime = 0;
	time = 0;
	framenum = 0;

	spawnArgs.Clear();

	clip->Init();
}

void idGameLocal::Clear()
{
	entities.clear();
	entities.resize(MAX_GENTITIES);
	firstFreeEntityIndex[0] = 0;
	firstFreeEntityIndex[1] = ENTITYNUM_FIRST_NON_REPLICATED;
	num_entities = 0;
	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;
	
	if (clip)
	{
		clip->Shutdown();
		clip = nullptr;
	}

	framenum = 0;
	previousTime = 0;
	time = 0;
	mapFileName.clear();
	spawnArgs.Clear();
	gamestate = GAMESTATE_UNINITIALIZED;

	ResetSlowTimeVars();

	colors.clear();
}

/*
==============
idGameLocal::GameState

Used to allow entities to know if they're being spawned during the initial spawn.
==============
*/
gameState_t	idGameLocal::GameState() const {
	return gamestate;
}

void idGameLocal::Shell_Show(bool show)
{
	tr.screen.setStdOutputBuffer();

	tr.screen.writeConsoleOutput("enter Q to quit or any key to continue: ");
}

void idGameLocal::SpawnMapEntities()
{
}

void idGameLocal::MapPopulate()
{
	// parse the key/value pairs and spawn entities
	SpawnMapEntities();

	// execute pending events before the very first game frame
	// this makes sure the map script main() function is called
	// before the physics are run so entities can bind correctly
	Printf("==== Processing events ====\n");
	idEvent::ServiceEvents();

	for(int i = 0; i < 1; ++i)
		AddRandomPoint();
}

void idGameLocal::MapClear(bool clearClients)
{
	for (size_t i = (clearClients ? 0 : MAX_CLIENTS); i < MAX_GENTITIES; i++) {
		auto ent = entities[i];

		if (ent) {
			ent->Remove();
		}

	}
}

void idGameLocal::AddRandomPoint()
{
	idDict args;

	size_t ent_type = GetRandomValue(0, 0);

	float searching_radius = 0.0f;
	float ent_size = 1.0f;
	float start_pos = 0.0f;

	if (ent_type == 0) {
		args.Set("classname", "idStaticEntity");
		args.Set("spawnclass", "idStaticEntity");
	}
	else if (ent_type == 1) {
		args.Set("classname", "idChain");
		args.Set("spawnclass", "idChain");

		size_t links = GetRandomValue(3, 10);
		args.Set("links", std::to_string(links));
		searching_radius = static_cast<float>(links);
		ent_size = searching_radius;
		start_pos = searching_radius;
	}
	else {
		args.Set("classname", "idSimpleObject");
		args.Set("spawnclass", "idSimpleObject");
	}

	Vector2 origin(GetRandomValue(start_pos, GetHeight() - ent_size), GetRandomValue(start_pos, GetWidth() - ent_size));
	//Vector2 origin = { 10.0f, 0.16f };
	Vector2 axis(0, 0);

	std::vector<std::shared_ptr<idEntity>> ent_vec(1);

	int num_attempts = 0;
	int finded_ents = 0;
	while ((finded_ents = EntitiesWithinRadius(origin, searching_radius, ent_vec, ent_vec.size())) != 0)
	{
		if (num_attempts++ > 100)
		{
			//Warning("couldn't spawn random point at %5.2f %5.2f, finded %d with radius %f", origin.x, origin.y, finded_ents, searching_radius);
			return;
		}

		origin = Vector2(GetRandomValue(start_pos, GetHeight() - ent_size), GetRandomValue(start_pos, GetWidth() - ent_size));
	}

	args.Set("origin", origin.ToString());
	args.Set("axis", axis.ToString());
	args.Set("model", "asterisk");
	args.Set("clipmodel", "pixel");
	args.Set("color", std::to_string(gameLocal.GetRandomColor()));
	args.Set("linearVelocity", Vector2(gameLocal.GetRandomValue(-2.0f, 2.0f), gameLocal.GetRandomValue(-10.0f, 10.0f)).ToString());
	//args.Set("linearVelocity", (Vector2(0.0f, -10.0f).ToString()));

	std::shared_ptr<idEntity> ent;
	if (!gameLocal.SpawnEntityDef(args, &ent)) {
		Warning("Failed to spawn random point as '%s'", args.GetString("classname").c_str());
	}
	else {
		ent->PostEventMS(&EV_Remove, 100000);
	}


	/*args.Set("classname", "idSimpleObject");
	args.Set("spawnclass", "idSimpleObject");
	args.Set("origin", Vector2(4.0f, 5.0f).ToString());
	args.Set("linearVelocity", (Vector2(0.0f, 0.0f).ToString()));
	args.Set("color", std::to_string(Screen::ConsoleColor::LightRed));
	gameLocal.SpawnEntityDef(args, &ent);*/
}

Screen::ConsoleColor idGameLocal::GetRandomColor()
{
	std::uniform_int_distribution<int> u_c(0, colors.size() - 1);

	int col(u_c(rand_eng));

	return colors[col];
}

bool idGameLocal::SpawnEntityDef(const idDict & args, std::shared_ptr<idEntity>* ent)
{
	std::string classname, spawn, error, name;
	idTypeInfo *cls;
	std::shared_ptr<idClass> obj;

	spawnArgs = args;

	if (spawnArgs.GetString("name", "", &name)) {
		sprintf(error, " on '%s'", name.c_str());
	}

	spawnArgs.GetString("classname", "", &classname);

	// check if we should spawn a class object
	spawnArgs.GetString("spawnclass", "", &spawn);
	if (!spawn.empty()) {

		cls = idClass::GetClass(spawn);
		if (!cls)
		{
			Warning("Could not spawn '%s'.  Class '%s' not found%s.", classname.c_str(), spawn.c_str(), error.c_str());
			return false;
		}

		obj = cls->CreateInstance();
		if (!obj)
		{
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

void idGameLocal::RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict & spawnArgsToCopy)
{
	int spawn_entnum;

	if (spawnCount >= (1 << (32 - GENTITYNUM_BITS))) {
		Error("idGameLocal::RegisterEntity: spawn count overflow");
	}

	if (!spawnArgsToCopy.GetInt("spawn_entnum", "0", spawn_entnum))
	{
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
	//spawnIds[spawn_entnum] = (forceSpawnId >= 0) ? forceSpawnId : spawnCount++;
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

void idGameLocal::UnregisterEntity(std::shared_ptr<idEntity> ent)
{
	if ((ent->entityNumber != ENTITYNUM_NONE) && (entities[ent->entityNumber] == ent)) {
		ent->spawnNode.Remove();
		entities[ent->entityNumber] = nullptr;

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
Vector2 idGameLocal::SelectInitialSpawnPoint(std::shared_ptr<idPlayer> player) {
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
void idGameLocal::SelectTimeGroup(int timeGroup) {
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
int idGameLocal::GetTimeGroupTime(int timeGroup) {
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
void idGameLocal::ResetSlowTimeVars() {
	//slowmoScale = 1.0f;
	//slowmoState = SLOWMO_STATE_OFF;

	fast.previousTime = 0;
	fast.time = 0;

	slow.previousTime = 0;
	slow.time = 0;
}