#include <exception>

#include "Game_local.h"
#include "Class.h"
#include "RenderWorld_local.h"
#include "Common.h"

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
	Clear();

	idClass::Init();

	height = 10;
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

	rand_eng.seed(static_cast<unsigned>(std::time(0)));

	/*unsigned snakeSize = 3;

	Vector2 origin(GetRandomValue(0U, width - 1), GetRandomValue(0U, height - 1));
	Vector2 axis(0, 0);

	idDict args;

	args.Set("classname", "Player");
	args.Set("spawnclass", "Player");
	args.Set("origin", origin.ToString());
	args.Set("axis", axis.ToString());
	args.Set("color", std::to_string(GetRandomColor()));

	SpawnEntityDef(args);*/
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

	MapShutdown();

	idClass::Shutdown();

	// free memory allocated by class objects
	Clear();
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
	gamestate = GAMESTATE_SHUTDOWN;

	MapClear(true);

	mapFileName.clear();

	gameRenderWorld = nullptr;

	gamestate = GAMESTATE_NOMAP;
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

	SelectTimeGroup(false);

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
}

bool idGameLocal::Draw(int clientNum)
{
	tr.DrawFPS();
	gameRenderWorld->RenderScene(nullptr);
	tr.console.clear();

	return true;
}

void idGameLocal::LoadMap(const std::string mapName, int randseed)
{
	mapFileName = mapName;

	entities.clear();
	entities.resize(MAX_GENTITIES);
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
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;
	framenum = 0;
	previousTime = 0;
	time = 0;
	colors.clear();
	spawnArgs.Clear();
	mapFileName.clear();
	gamestate = GAMESTATE_UNINITIALIZED;

	ResetSlowTimeVars();
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
	for (size_t i = (clearClients ? 0 : MAX_CLIENTS); i < MAX_GENTITIES; i++)
	{
		auto ent = entities[i];

		if (ent)
		{
			ent->activeNode.SetOwner(nullptr);
			gameLocal.UnregisterEntity(ent);
			ent->GetPhysics()->SetSelf(nullptr);
		}

	}
}

void idGameLocal::AddRandomPoint()
{
	Vector2 origin(GetRandomValue(0U, width - 1), GetRandomValue(0U, height - 1));
	Vector2 axis(0, 0);

	idDict args;

	args.Set("classname", "idStaticEntity");
	args.Set("spawnclass", "idStaticEntity");
	args.Set("origin", origin.ToString());
	args.Set("axis", axis.ToString());
	args.Set("model", "pixel");
	args.Set("color", std::to_string(GetRandomColor()));

	SpawnEntityDef(args);

	size_t numIters = 0;

	/*while (!checkCollidePosToAllObjects(pos))
	{
		pos = { u_h(rand_eng), u_w(rand_eng) };

		if (++numIters == 10)
			break;
	}*/

	//auto point = new Point(pos, Screen::Pixel('*', GetRandomColor()));

	//AddObject(point);
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
		throw std::range_error("idGameLocal::RegisterEntity: spawn count overflow");
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
			std::range_error("no free entities");
		}
		spawn_entnum = freeIndex++;

		firstFreeEntityIndex[freeListType] = freeIndex;
	}

	entities[spawn_entnum] = ent;
	ent->entityNumber = spawn_entnum;

	// Make a copy because TransferKeyValues clears the input parameter.
	idDict copiedArgs = spawnArgsToCopy;
	//ent->spawnArgs.TransferKeyValues(copiedArgs);

	ent->spawnArgs = copiedArgs;
}

void idGameLocal::UnregisterEntity(std::shared_ptr<idEntity> ent)
{
	if ((ent->entityNumber != ENTITYNUM_NONE) && (entities[ent->entityNumber] == ent))
	{
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