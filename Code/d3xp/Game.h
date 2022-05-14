#ifndef GAME_H
#define GAME_H

struct gameReturn_t {
	std::string sessionCommand;	// "map", "disconnect", "victory", etc
};

class idGame {
public:
	idGame() = default;
	virtual ~idGame() = default;
	idGame(const idGame&) = default;
	idGame& operator=(const idGame&) = default;
	idGame(idGame&&) = default;
	idGame& operator=(idGame&&) = default;

	// Initialize the game for the first time.
	virtual void Init() = 0;

	// Shut down the entire game.
	virtual void Shutdown() = 0;

	// Loads a map and spawns all the entities.
	virtual void InitFromNewMap(const std::string& mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) = 0;

	// Shut down the current map.
	virtual void MapShutdown() = 0;

	// Caches media referenced from in key/value pairs in the given dictionary.
	virtual void CacheDictionaryMedia(gsl::not_null<const idDict*> dict) = 0;

	// Runs a game frame, may return a session command for level changing, etc
	virtual void RunFrame(gameReturn_t& ret) = 0;

	// Makes rendering and sound system calls to display for a given clientNum.
	virtual bool Draw(int clientNum) = 0;

	virtual bool IsInGame() const = 0;

	virtual int GetLocalClientNum() const = 0;

	virtual int GetTime() const = 0;

	// MAIN MENU FUNCTIONS
	virtual void Shell_Init(const std::string& filename) = 0;
	virtual void Shell_Cleanup() = 0;
	virtual void Shell_CreateMenu(bool inGame) = 0;
	virtual void Shell_Show(bool show) = 0;
	virtual bool Shell_IsActive() const = 0;
	virtual bool Shell_HandleGuiEvent(const sysEvent_t* sev) = 0;
	virtual void Shell_Render() = 0;
	virtual void Shell_ResetMenu() = 0;
	virtual void Shell_SyncWithSession() = 0;
};

extern idGame* game;

class idGameEdit {
public:
	idGameEdit() = default;
	virtual ~idGameEdit() = default;
	idGameEdit(const idGameEdit&) = default;
	idGameEdit& operator=(const idGameEdit&) = default;
	idGameEdit(idGameEdit&&) = default;
	idGameEdit& operator=(idGameEdit&&) = default;

	virtual void ParseSpawnArgsToRenderEntity(gsl::not_null<const idDict*> args, renderEntity_t* renderEntity);
};

extern idGameEdit* gameEdit;

#endif
