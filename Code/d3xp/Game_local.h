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

	void Set(game_time_type t, game_time_type pt/*, int rct*/) noexcept { time = t; previousTime = pt;/* realClientTime = rct;*/ };
	void Get(game_time_type& t, game_time_type& pt/*, int & rct*/) noexcept { t = time; pt = previousTime;/* rct = realClientTime;*/ };
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

	idClip clip; // collision detection

	std::string sessionCommand; // a target_sessionCommand can set this to return something to the session

	int framenum;
	game_time_type time;		// in msec
	int	previousTime;			// time in msec of last frame
	game_time_type prev_info_update_time;

	timeState_t fast;
	timeState_t slow;
	int						selectedGroup;

	virtual void SelectTimeGroup(int timeGroup) noexcept;
	virtual int GetTimeGroupTime(int timeGroup) noexcept;

	void ResetSlowTimeVars() noexcept;

	// ---------------------- Public idGame Interface -------------------

	idGameLocal();
	~idGameLocal();
	idGameLocal(const idGameLocal&) = default;
	idGameLocal& operator=(const idGameLocal&) = default;
	idGameLocal(idGameLocal&&) = default;
	idGameLocal& operator=(idGameLocal&&) = default;

	void Init() override;
	void Shutdown() override;

	void InitFromNewMap(const std::string &mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) override;
	void MapShutdown() override;
	void CacheDictionaryMedia(gsl::not_null<const idDict*> dict) override;
	void RunFrame(idUserCmdMgr& cmdMgr, gameReturn_t& ret) override;
	void RunAllUserCmdsForPlayer(idUserCmdMgr& cmdMgr, const int playerNumber);
	void RunSingleUserCmd(usercmd_t& cmd, gsl::not_null<idPlayer*> player);
	void RunEntityThink(idEntity& ent, idUserCmdMgr& userCmdMgr);
	bool Draw(int clientNum) override;
	void BuildReturnValue(gameReturn_t& ret);

	bool IsInGame() const noexcept override  { return GameState() == GAMESTATE_ACTIVE; }

	int GetLocalClientNum() const noexcept override;

	// ---------------------- Public idGameLocal Interface -------------------

	void Printf(const char* fmt, ...) const;
	void DPrintf(const char* fmt, ...) const;
	void Warning(const char* fmt, ...) const;
	void DWarning(const char* fmt, ...) const;
	void Error(const char* fmt, ...) const;

	// Initializes all map variables common to both save games and spawned games
	void LoadMap(const std::string& mapName, int randseed);

	bool CheatsOk(bool requirePlayer = true);
	gameState_t GameState() const noexcept;
	std::shared_ptr<idEntity> SpawnEntityType(const idTypeInfo& classdef, const idDict* args = nullptr);
	bool SpawnEntityDef(const idDict &args, std::shared_ptr<idEntity> *ent = nullptr);

	const std::shared_ptr<idDeclEntityDef> FindEntityDef(const std::string& name, bool makeDefault = true) const;
	const idDict* FindEntityDefDict(const std::string& name, bool makeDefault = true) const;

	void RegisterEntity(std::shared_ptr<idEntity> ent, int forceSpawnId, const idDict & spawnArgsToCopy);
	void UnregisterEntity(std::shared_ptr<idEntity> ent) noexcept;
	const idDict &GetSpawnArgs() const noexcept { return spawnArgs; }

	int EntitiesWithinRadius(const Vector2 org, float radius, std::vector<std::shared_ptr<idEntity>>& entityList, int maxCount) const;

	// added the following to assist licensees with merge issues
	int GetFrameNum() const noexcept { return framenum; };
	int GetTime() const noexcept override { return time; };

	idPlayer* GetLocalPlayer() const;

	Vector2 SelectInitialSpawnPoint(idPlayer* player);

	void SyncPlayersWithLobbyUsers(bool initial);

	bool InhibitControls() override;

	// MAIN MENU FUNCTIONS
	void Shell_Init(const std::string& filename) override;
	void Shell_Cleanup() noexcept override;
	void Shell_Show(bool show) noexcept override;
	void Shell_CreateMenu(bool inGame) override;
	bool Shell_IsActive() const noexcept override;
	bool Shell_HandleGuiEvent(const sysEvent_t* sev) override;
	void Shell_Render() override;
	void Shell_ResetMenu() override;
	void Shell_SyncWithSession() override;

	template <typename T>
	T GetRandomValue(T min_val, T max_val);
	template <typename T>
	T GetRandomValue(std::initializer_list<T> values);

	Screen::color_type GetRandomColor();

	short GetHeight() noexcept { return height; }
	short GetWidth() noexcept { return width; }

	const std::vector<Screen::color_type>& GetColors() const noexcept { return colors; }
	void AddRandomPoint();

	int GetInfoUpdateTime() noexcept { return info_update_time; }
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
	void MapClear(bool clearClients) noexcept;

	void RunDebugInfo();
	void PrintSpawnedEntities();

	void SpawnPlayer(int clientNum);

	void InitConsoleCommands();
	void ShutdownConsoleCommands();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	short height, width;
	int info_update_time;
	std::vector<Screen::color_type> colors;

	std::default_random_engine rand_eng;

	bool menu_active;
};

template<typename T>
inline T idGameLocal::GetRandomValue(T min_val, T max_val) {
	T cur_min = std::max(min_val, static_cast<T>(0));

	if (min_val < 0)
		max_val = max_val - min_val;

	if (min_val > max_val)
		Warning("Error getting random value. Min %3f, max %3f", min_val, max_val);

	std::uniform_int_distribution<size_t> u(static_cast<size_t>(cur_min), static_cast<size_t>(max_val));

	T cur_val = static_cast<T>(u(rand_eng));

	if (min_val < 0) {
		cur_val += min_val;
	}

	return cur_val;
}

template <typename T>
T idGameLocal::GetRandomValue(std::initializer_list<T> values) {
	return *std::next(values.begin(), GetRandomValue(static_cast<typename std::initializer_list<T>::size_type>(0), values.size() - 1));
	return 0;
}

extern idGameLocal gameLocal;

// content masks
constexpr auto MASK_ALL = (-1);
constexpr auto MASK_SOLID = static_cast<int>(contentsFlags_t::CONTENTS_SOLID);
constexpr auto MASK_MONSTERSOLID = static_cast<int>(contentsFlags_t::CONTENTS_SOLID) | static_cast<int>(contentsFlags_t::CONTENTS_MONSTERCLIP)
| static_cast<int>(contentsFlags_t::CONTENTS_BODY);
constexpr auto MASK_PLAYERSOLID = static_cast<int>(contentsFlags_t::CONTENTS_SOLID) | static_cast<int>(contentsFlags_t::CONTENTS_PLAYERCLIP)
| static_cast<int>(contentsFlags_t::CONTENTS_BODY);
constexpr auto MASK_DEADSOLID = static_cast<int>(contentsFlags_t::CONTENTS_SOLID) | static_cast<int>(contentsFlags_t::CONTENTS_PLAYERCLIP);

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