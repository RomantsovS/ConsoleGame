#include <exception>

#include "Game_local.h"
#include "../d3xp/gamesys/Class.h"
#include "../renderer/RenderWorld_local.h"
#include "../framework/Common.h"
#include "../idlib/Lib.h"
#include "../idlib/Str.h"

std::shared_ptr<idRenderWorld> gameRenderWorld; // all drawing is done to this world

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
idGameLocal gameLocal;
idGame *game = &gameLocal;

idGameLocal::idGameLocal()
{
	Clear();
}

void idGameLocal::Init()
{
	Printf("--------- Initializing Game ----------\n");
	Printf("gamename: %s\n", GAME_VERSION);
	Printf("gamedate: %s\n", __DATE__);

	Clear();

	clip = std::make_shared<idClip>();

	idClass::Init();

	height = 16;
	width = 50;

	colors.push_back(Screen::Green);
	colors.push_back(Screen::Cyan);
	colors.push_back(Screen::Red);
	colors.push_back(Screen::Magenta);
	colors.push_back(Screen::Brown);
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

	// free the collision map
	collisionModelManager->FreeMap();

	idClass::Shutdown();

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

	gamestate = GAMESTATE_STARTUP;

	gameRenderWorld = renderWorld;

	LoadMap(mapName, randseed);

	MapPopulate();

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

void idGameLocal::RunFrame()
{
	if (!gameRenderWorld)
	{
		return;
	}

	// update the game time
	framenum++;
	fast.previousTime = FRAME_TO_MSEC(framenum - 1);
	fast.time = FRAME_TO_MSEC(framenum);
	fast.realClientTime = fast.time;

	//ComputeSlowScale();

	/*slow.previousTime = slow.time;
	slow.time += idMath::Ftoi((fast.time - fast.previousTime) * slowmoScale);
	slow.realClientTime = slow.time;*/

	SelectTimeGroup(true);

	// make sure the random number counter is used each frame so random events
	// are influenced by the player's actions
	rand_eng.seed(Sys_Time());

	// clear any debug lines from a previous frame
	gameRenderWorld->DebugClearLines(time);

	static auto lastTimePointSpawn = time;
	if (time - lastTimePointSpawn > 10000) {
		lastTimePointSpawn = time;
		AddRandomPoint();
	}

	// let entities think
	for (auto ent = activeEntities.Next(); ent; ent = ent->activeNode.Next())
	{
		ent->Think();
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

	// show any debug info for this frame
	RunDebugInfo();
}

bool idGameLocal::Draw(int clientNum)
{
	tr.console += std::to_string(time) + " ";
	tr.DrawFPS();

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
	std::shared_ptr<idEntity> ent;
	/*idPlayer *player;

	player = GetLocalPlayer();
	if (!player) {
		return;
	}

	const idVec3 &origin = player->GetPhysics()->GetOrigin();*/

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
			if (ent->IsActive())
			{
				char buf[256];
				sprintf_s(buf, "ent %s %s pos [%2.3f %2.3f] vel [%2.3f %2.3f] rest %d", ent->GetName().c_str(),
					ent->GetClassname().c_str(), ent->GetPhysics()->GetOrigin().x,
					ent->GetPhysics()->GetOrigin().y, ent->GetPhysics()->GetLinearVelocity().x,
					ent->GetPhysics()->GetLinearVelocity().y, ent->GetPhysics()->IsAtRest());

				gameRenderWorld->DrawText(buf, Vector2(), ent->GetRenderEntity()->color, 1);
			}
		}
	}

	gameRenderWorld->DrawText(std::to_string(num_entities - MAX_CLIENTS), Vector2(), Screen::ConsoleColor::Yellow, 0);
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

	clip->DrawClipSectors();

	/*if (g_showCollisionModels.GetBool()) {
		clip.DrawClipModels(player->GetEyePosition(), g_maxShowDistance.GetFloat(), pm_thirdPerson.GetBool() ? NULL : player);
	}

	if (g_showCollisionTraces.GetBool()) {
		clip.PrintStatistics();
	}

	if (g_showPVS.GetInteger()) {
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

/*
void Game::onKeyPressed(char c)
{
	switch (c)
	{
	case 72: case 'w':
	{
		onMoveKeyPressed(SimpleObject::UP);

		break;
	}
	case 80: case 's':
	{
		onMoveKeyPressed(SimpleObject::DOWN);

		break;
	}
	case 75: case 'a':
	{
		onMoveKeyPressed(SimpleObject::LEFT);

		break;
	}
	case 77: case 'd':
	{
		onMoveKeyPressed(SimpleObject::RIGHT);

		break;
	}
	}
}*/
/*
void idGameLocal::BreakTime()
{
	clock_t temp;
	temp = clock() + delayMilliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < temp)
	{
	}
}*/

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

void idGameLocal::SpawnMapEntities()
{
}

void idGameLocal::MapPopulate()
{
	// parse the key/value pairs and spawn entities
	SpawnMapEntities();

	AddRandomPoint();

	//AddRandomPoint();
}

void idGameLocal::MapClear(bool clearClients)
{
	for (size_t i = (clearClients ? 0 : MAX_CLIENTS); i < MAX_GENTITIES; i++) {
		auto ent = entities[i];

		if (ent)
		{
			//ent->activeNode.SetOwner(nullptr);
			//ent->spawnNode.SetOwner(nullptr);
			ent->FreeModelDef();
			gameLocal.UnregisterEntity(ent);
			//ent->GetPhysics()->SetSelf(nullptr);
		}

	}
}

void idGameLocal::AddRandomPoint()
{
	Vector2 origin(GetRandomValue(0.0f, GetHeight() - 1.0f), GetRandomValue(0.0f, GetWidth() - 1.0f));
	//Vector2 origin(0.0f, 0.0f);
	Vector2 axis(0, 0);

	idDict args;

	args.Set("classname", "idSimpleObject");
	args.Set("spawnclass", "idSimpleObject");
	//args.Set("classname", "idStaticEntity");
	//args.Set("spawnclass", "idStaticEntity");
	args.Set("origin", origin.ToString());
	args.Set("axis", axis.ToString());
	args.Set("model", "pixel");
	args.Set("color", std::to_string(GetRandomColor()));
	args.Set("linearVelocity", (Vector2(GetRandomValue(0.0f, 100.0f) / 1000, GetRandomValue(0.0f, 100.0f) / 1000).ToString()));

	SpawnEntityDef(args);

	size_t numIters = 0;

	/*while (!checkCollidePosToAllObjects(pos))
	{
		pos = { u_h(rand_eng), u_w(rand_eng) };

		if (++numIters == 10)
			break;
	}*/
}

/*
bool Game::checkCollideObjects(SimpleObject *object)
{
	for (auto iter = collideObjects.begin(); iter != collideObjects.end();)
	{
		auto obj = *iter;

		if (object == obj.get())
		{
			++iter;
			continue;
		}


		if (!object->checkCollide(std::static_pointer_cast<Point>(obj)))
		{
			obj->setActive(false);

			addRandomPoint();
			addRandomPoint();

			delayMilliseconds -= 10;

			return false;
		}
		else
			++iter;
	}

	return true;
}*/

/*
void Game::onMoveKeyPressed(SimpleObject::directions dir)
{
	snake->setDirection(dir);

	snake->think(*this);
}*/

/*
bool Game::checkCollidePosToAllObjects(pos_type pos)
{
	for (auto iter = collideObjects.cbegin(); iter != collideObjects.cend(); ++iter)
	{
		auto obj = *iter;

		if (!obj->checkCollide(pos))
			return false;
	}

	return true;
}*/

Screen::ConsoleColor idGameLocal::GetRandomColor()
{
	std::uniform_int_distribution<int> u_c(0, colors.size() - 1);

	int col(u_c(rand_eng));

	return colors[col];
}

bool idGameLocal::SpawnEntityDef(const idDict & args, std::shared_ptr<idEntity> ent)
{
	std::string className, spawn;
	idTypeInfo *cls;
	std::shared_ptr<idClass> obj;

	spawnArgs = args;

	spawnArgs.GetString("classname", "", &className);

	spawnArgs.GetString("spawnclass", "", &spawn);

	if (!spawn.empty()) {

		cls = idClass::GetClass(spawn);
		if (!cls)
		{
			return false;
		}

		obj = cls->CreateInstance();
		if (!obj)
		{
			return false;
		}

		obj->CallSpawn();

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
			Error("no free entities");
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
	if ((ent->entityNumber != ENTITYNUM_NONE) && (entities[ent->entityNumber] == ent))
	{
		ent->spawnNode.Remove();
		entities[ent->entityNumber] = nullptr;

		ent->entityNumber = ENTITYNUM_NONE;
	}
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