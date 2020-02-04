#ifndef GAME_H
#define GAME_H

#include "Entity.h"
#include "Dict.h"

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

	// Runs a game frame, may return a session command for level changing, etc
	virtual void RunFrame() = 0;

	// Makes rendering and sound system calls to display for a given clientNum.
	virtual bool Draw(int clientNum) = 0;
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
