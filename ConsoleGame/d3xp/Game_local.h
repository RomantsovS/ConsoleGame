#ifndef GAME_LOCAL_H
#define GAME_LOCAL_H

extern std::shared_ptr<idRenderWorld> gameRenderWorld;
extern idCVar game_width;
extern idCVar game_height;

// the "gameversion" client command will print this plus compile date
const std::string GAME_VERSION = "baseDOOM-1";

// classes used by idGameLocal
class idEntity;
class idActor;
class idPlayer;
class idWorldspawn;
class idAI;
class idTypeInfo;
class idMenuHandler_Shell;

const int MAX_CLIENTS = 8;
const int GENTITYNUM_BITS = 12;
const int MAX_GENTITIES = 1 << GENTITYNUM_BITS;
const int ENTITYNUM_NONE = MAX_GENTITIES - 1;
const int ENTITYNUM_WORLD = MAX_GENTITIES - 2;
const int ENTITYNUM_MAX_NORMAL = MAX_GENTITIES - 2;
const int ENTITYNUM_FIRST_NON_REPLICATED = ENTITYNUM_MAX_NORMAL - 256;

#include "gamesys/Event.h"
#include "gamesys/Class.h"
#include "gamesys/SysCvar.h"

#include "anim/Anim.h"

#include "physics/Clip.h"

enum gameState_t {
	GAMESTATE_UNINITIALIZED,		// prior to Init being called
	GAMESTATE_NOMAP,				// no map loaded
	GAMESTATE_STARTUP,				// inside InitFromNewMap().  spawning map entities.
	GAMESTATE_ACTIVE,				// normal gameplay
	GAMESTATE_SHUTDOWN				// inside MapShutdown().  clearing memory.
};

using game_time_type = int;

struct timeState_t {
	game_time_type time;
	game_time_type previousTime;
	game_time_type realClientTime;

	void Set(game_time_type t, game_time_type pt/*, int rct*/) { time = t; previousTime = pt;/* realClientTime = rct;*/ };
	void Get(game_time_type& t, game_time_type& pt/*, int & rct*/) { t = time; pt = previousTime;/* rct = realClientTime;*/ };
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
	std::shared_ptr<idWorldspawn> world; // world entity

	std::shared_ptr<idClip> clip; // collision detection

	int framenum;
	game_time_type time;					// in msec
	int	previousTime;			// time in msec of last frame

	timeState_t fast;
	timeState_t slow;
	int						selectedGroup;

	virtual void SelectTimeGroup(int timeGroup);
	virtual int GetTimeGroupTime(int timeGroup);

	void ResetSlowTimeVars();

	// ---------------------- Public idGame Interface -------------------

	idGameLocal();
	~idGameLocal();

	virtual void Init() override;
	void Shutdown() override;

	void InitFromNewMap(const std::string &mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) override;
	virtual void MapShutdown() override;
	virtual void CacheDictionaryMedia(const idDict* dict) override;
	virtual void RunFrame() override;
	void RunAllUserCmdsForPlayer(/*idUserCmdMgr& cmdMgr,*/ const int playerNumber);
	void RunSingleUserCmd(usercmd_t& cmd, std::shared_ptr<idPlayer>& player);
	void RunEntityThink(idEntity& ent/*, idUserCmdMgr& userCmdMgr*/);
	bool Draw(int clientNum) override;

	virtual bool IsInGame() const override  { return GameState() == GAMESTATE_ACTIVE; }

	virtual int GetLocalClientNum() const override;

	// ---------------------- Public idGameLocal Interface -------------------

	void Printf(const char* fmt, ...) const;
	void DPrintf(const char* fmt, ...) const;
	void Warning(const char* fmt, ...) const;
	void DWarning(const char* fmt, ...) const;
	void Error(const char* fmt, ...) const;

	// Initializes all map variables common to both save games and spawned games
	void LoadMap(const std::string mapName, int randseed);

	gameState_t GameState() const;
	std::shared_ptr<idEntity> SpawnEntityType(const idTypeInfo& classdef, const std::shared_ptr<idDict>& args = nullptr);
	bool SpawnEntityDef(const idDict &args, std::shared_ptr<idEntity> *ent = nullptr);

	const std::shared_ptr<idDeclEntityDef> FindEntityDef(const std::string& name, bool makeDefault = true) const;
	const idDict* FindEntityDefDict(const std::string& name, bool makeDefault = true) const;

	void RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict & spawnArgsToCopy);
	void UnregisterEntity(std::shared_ptr<idEntity> ent);
	const idDict &GetSpawnArgs() const { return spawnArgs; }

	int EntitiesWithinRadius(const Vector2 org, float radius, std::vector<std::shared_ptr<idEntity>>& entityList, int maxCount) const;

	// added the following to assist licensees with merge issues
	int GetFrameNum() const { return framenum; };
	int GetTime() const { return time; };

	std::shared_ptr<idPlayer> GetLocalPlayer() const;

	Vector2 SelectInitialSpawnPoint(std::shared_ptr<idPlayer> player);

	void SyncPlayersWithLobbyUsers(bool initial);

	// MAIN MENU FUNCTIONS
	void Shell_Init(const std::string& filename) override;
	void Shell_Cleanup() override;
	void Shell_Show(bool show) override;
	void Shell_CreateMenu(bool inGame);
	bool Shell_IsActive() const override;
	bool Shell_HandleGuiEvent(const sysEvent_t* sev) override;
	void Shell_Render() override;
	void Shell_ResetMenu() override;
	void Shell_SyncWithSession() override;

	template <typename T>
	T GetRandomValue(T min, T max);

	Screen::ConsoleColor GetRandomColor();

	short GetHeight() { return height; }
	short GetWidth() { return width; }

	const std::vector<Screen::ConsoleColor>& GetColors() const { return colors; }
	void AddRandomPoint();
private:
	std::string mapFileName; // name of the map, empty string if no map loaded
	std::shared_ptr<idMapFile> mapFile; // will be NULL during the game unless in-game editing is used

	int spawnCount;
	
	std::shared_ptr<idMenuHandler_Shell> shellHandler;

	idDict spawnArgs;

	gameState_t gamestate; // keeps track of whether we're spawning, shutting down, or normal gameplay

	void Clear();
							// spawn entities from the map file
	void SpawnMapEntities();
	// commons used by init, shutdown, and restart
	void MapPopulate();
	void MapClear(bool clearClients);

	void RunDebugInfo();
	void RunDebugInfoScreen();
	void PrintSpawnedEntities();

	void SpawnPlayer(int clientNum);

	void InitConsoleCommands();
	void ShutdownConsoleCommands();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	short height, width;

	std::vector<Screen::ConsoleColor> colors;

	std::default_random_engine rand_eng;

	bool menu_active;
};

template<typename T>
inline T idGameLocal::GetRandomValue(T min, T max)
{
	T cur_min = max(min, 0);

	if (min < 0)
		max = max - min;

	if (min > max)
		Warning("Error getting random value. Min %3f, max %3f", min, max);

	std::uniform_int_distribution<size_t> u(static_cast<size_t>(cur_min), static_cast<size_t>(max));

	T cur_val = static_cast<T>(u(rand_eng));

	if (min < 0) {
		cur_val += min;
	}

	return cur_val;
}

extern idGameLocal gameLocal;

// content masks
const int MASK_SOLID = 1;

#include "physics/Physics.h"
#include "physics/Physics_Static.h"
#include "physics/Physics_Base.h"
#include "physics/Physics_Actor.h"
#include "physics/Physics_Player.h"
#include "physics/Physics_RigidBody.h"
#include "physics/Physics_AF.h"
#include "physics/Physics_PlayerChain.h"
#include "physics/Physics_PlayerMy.h"

#include "Entity.h"
#include "AFEntity.h"
#include "Misc.h"
#include "Actor.h"
#include "Projectile.h"
#include "Weapon.h"
#include "WorldSpawn.h"
#include "Player.h"
#include "PlayerChain.h"

#include "ai/AI.h"

// menus
#include "menus/MenuWidget.h"
#include "menus/MenuScreen.h"
#include "menus/MenuHandler.h"

#endif