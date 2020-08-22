#ifndef GAME_H
#define GAME_H

#include "../d3xp/Entity.h"
#include "../idlib/Dict.h"

class idGame
{
public:
	virtual ~idGame() {}

	// Initialize the game for the first time.
	virtual void Init() = 0;

	// Shut down the entire game.
	virtual void Shutdown() = 0;

	// Loads a map and spawns all the entities.
	virtual void InitFromNewMap(const std::string &mapName, std::shared_ptr<idRenderWorld> renderWorld, int randseed) = 0;

	// Shut down the current map.
	virtual void MapShutdown() = 0;

	// Runs a game frame, may return a session command for level changing, etc
	virtual void RunFrame() = 0;

	// Makes rendering and sound system calls to display for a given clientNum.
	virtual bool Draw(int clientNum) = 0;

	virtual bool IsInGame() const = 0;

	// MAIN MENU FUNCTIONS
	virtual bool Shell_IsActive() const = 0;
	virtual void Shell_Show(bool show) = 0;
};

extern idGame *game;

class idGameEdit
{
public:
	virtual ~idGameEdit() {}

	virtual void ParseSpawnArgsToRenderEntity(const idDict *args, renderEntity_t *renderEntity);
};

extern idGameEdit *gameEdit;

#endif
