#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

#include <random>
#include <array>

#include "Game.h"
#include "../renderer/RenderWorld.h"
#include "physics/Clip.h"
#include "../idlib/containers/LinkList.h"

extern std::shared_ptr<idRenderWorld> gameRenderWorld;

// the "gameversion" client command will print this plus compile date
const std::string GAME_VERSION = "baseDOOM-1";

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

struct timeState_t {
	int					time;
	int					previousTime;
	int					realClientTime;

	void				Set(int t, int pt/*, int rct*/) { time = t; previousTime = pt;/* realClientTime = rct;*/ };
	void				Get(int & t, int & pt/*, int & rct*/) { t = time; pt = previousTime;/* rct = realClientTime;*/ };
	//void				Save(idSaveGame *savefile) const { savefile->WriteInt(time); savefile->WriteInt(previousTime); savefile->WriteInt(realClientTime); }
	//void				Restore(idRestoreGame *savefile) { savefile->ReadInt(time); savefile->ReadInt(previousTime); savefile->ReadInt(realClientTime); }
};

class idGameLocal : public idGame {
public:
	std::vector<std::shared_ptr<idEntity>> entities;
	std::array<int, 2> firstFreeEntityIndex;	// first free index in the entities array. [0] for replicated entities, [1] for non-replicated
	idLinkList<idEntity> spawnedEntities; // all spawned entities
	idLinkList<idEntity> activeEntities; // all thinking entities (idEntity::thinkFlags != 0)
	int numEntitiesToDeactivate; // number of entities that became inactive in current frame
	int num_entities; // current number <= MAX_GENTITIES

	std::shared_ptr<idClip> clip; // collision detection

	int framenum;
	int time;					// in msec
	int	previousTime;			// time in msec of last frame

	timeState_t fast;
	timeState_t slow;
	int						selectedGroup;

	virtual void SelectTimeGroup(int timeGroup);
	virtual int GetTimeGroupTime(int timeGroup);

	void ResetSlowTimeVars();

	// ---------------------- Public idGame Interface -------------------

	idGameLocal();

	virtual void Init() override;
	virtual void Shutdown() override;

	virtual void InitFromNewMap(const std::string &mapName, std::shared_ptr<idRenderWorld> renderWorld,
		int randseed) override;
	virtual void MapShutdown() override;

	virtual void RunFrame() override;

	virtual bool Draw(int clientNum) override;

	// ---------------------- Public idGameLocal Interface -------------------

	void Printf(const char* fmt, ...) const;
	void DPrintf(const char* fmt, ...) const;
	void Warning(const char* fmt, ...) const;
	void DWarning(const char* fmt, ...) const;
	void Error(const char* fmt, ...) const;

	// Initializes all map variables common to both save games and spawned games
	void LoadMap(const std::string mapName, int randseed);

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/

	template <typename T>
	T GetRandomValue(T min, T max);

	Screen::ConsoleColor GetRandomColor();

	bool SpawnEntityDef(const idDict &args, std::shared_ptr<idEntity> ent = nullptr);

	void RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict & spawnArgsToCopy);
	void UnregisterEntity(std::shared_ptr<idEntity> ent);

	const idDict &GetSpawnArgs() const { return spawnArgs; }

	short GetHeight() { return height; }
	short GetWidth() { return width; }
private:
	std::string mapFileName; // name of the map, empty string if no map loaded
	//idMapFile* mapFile;				// will be NULL during the game unless in-game editing is used

	int spawnCount;
	
	idDict spawnArgs;

	gameState_t gamestate; // keeps track of whether we're spawning, shutting down, or normal gameplay

	void Clear();
							// spawn entities from the map file
	void SpawnMapEntities();
	// commons used by init, shutdown, and restart
	void MapPopulate();
	void MapClear(bool clearClients);

	void RunDebugInfo();

	void AddRandomPoint();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	short height, width;

	std::vector<Screen::ConsoleColor> colors;

	std::default_random_engine rand_eng;
};

template<typename T>
inline T idGameLocal::GetRandomValue(T min, T max)
{
	T cur_min = max(min, 0);

	if (min < 0)
		max = max - min;

	std::uniform_int_distribution<size_t> u(static_cast<size_t>(cur_min), static_cast<size_t>(max));

	T cur_val = static_cast<T>(u(rand_eng));

	if (min < 0)
	{
		cur_val += min;
	}

	return cur_val;
}

extern idGameLocal gameLocal;

// content masks
const int MASK_SOLID = 1;

#endif