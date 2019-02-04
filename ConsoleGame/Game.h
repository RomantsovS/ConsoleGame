#ifndef Game_H
#define Game_H

#include <list>
#include <ctime>
#include <random>

#include "Entity.h"
#include "tr_local.h"
#include "Class.h"
#include "Dict.h"

class Game
{
public:
	virtual ~Game() {}

	virtual void Init() = 0;
};

extern Game *game;

class GameEdit
{
public:
	virtual ~GameEdit() {}

	virtual void ParseSpawnArgsToRenderEntity(const Dict *args, renderEntity_s *renderEntity);
};

extern GameEdit *gameEdit;

#endif
