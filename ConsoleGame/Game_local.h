#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

#include <random>
#include <array>

#include "Game.h"
#include "RenderWorld.h"

extern std::shared_ptr<idRenderWorld> gameRenderWorld;

const int MAX_CLIENTS = 8;
const int GENTITYNUM_BITS = 12;
const int MAX_GENTITIES = 1 << GENTITYNUM_BITS;
const int ENTITYNUM_NONE = MAX_GENTITIES - 1;
const int ENTITYNUM_WORLD = MAX_GENTITIES - 2;
const int ENTITYNUM_MAX_NORMAL = MAX_GENTITIES - 2;
const int ENTITYNUM_FIRST_NON_REPLICATED = ENTITYNUM_MAX_NORMAL - 256;

enum gameState_t {
	GAMESTATE_UNINITIALIZED,		// prior to Init being called
	GAMESTATE_NOMAP,				// no map loaded
	GAMESTATE_STARTUP,				// inside InitFromNewMap().  spawning map entities.
	GAMESTATE_ACTIVE,				// normal gameplay
	GAMESTATE_SHUTDOWN				// inside MapShutdown().  clearing memory.
};

class idGameLocal : public idGame
{
public:
	std::vector<std::shared_ptr<idEntity>> entities;
	idLinkList<idEntity> activeEntities; // all thinking entities (idEntity::thinkFlags != 0)
	int numEntitiesToDeactivate; // number of entities that became inactive in current frame

	std::array<int, 2> firstFreeEntityIndex;	// first free index in the entities array. [0] for replicated entities, [1] for non-replicated
	int num_entities; // current number <= MAX_GENTITIES

	int framenum;
	int time;					// in msec
	int	previousTime;			// time in msec of last frame

	// ---------------------- Public idGame Interface -------------------

	idGameLocal();

	virtual void Init() override;
	virtual void Shutdown() override;

	virtual void InitFromNewMap(const std::string mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) override;
	virtual void MapShutdown();

	virtual void RunFrame() override;

	virtual bool Draw(int clientNum) override;

	// ---------------------- Public idGameLocal Interface -------------------

	// Initializes all map variables common to both save games and spawned games
	void LoadMap(const std::string mapName, int randseed);

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/

	template <typename T>
	T GetRandomValue(T min, T max);

	Screen::ConsoleColor GetRandomColor();

	bool SpawnEntityDef(const idDict &args);

	void RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict & spawnArgsToCopy);
	void UnregisterEntity(std::shared_ptr<idEntity> ent);

	const idDict &GetSpawnArgs() const { return spawnArgs; }

	size_t GetHeight() { return height; }
	size_t GetWidth() { return width; }
private:
	std::string mapFileName; // name of the map, empty string if no map loaded

	int spawnCount;

	gameState_t gamestate; // keeps track of whether we're spawning, shutting down, or normal gameplay

	void Clear();
							// spawn entities from the map file
	void SpawnMapEntities();
	// commons used by init, shutdown, and restart
	void MapPopulate();
	void MapClear(bool clearClients);

	void AddRandomPoint();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	idDict spawnArgs;

	size_t height, width;

	std::vector<Screen::ConsoleColor> colors;

	std::default_random_engine rand_eng;
};

template<typename T>
inline T idGameLocal::GetRandomValue(T min, T max)
{
	std::uniform_int_distribution<Screen::pos> u(min, max);

	return u(rand_eng);
}

extern idGameLocal gameLocal;

#endif