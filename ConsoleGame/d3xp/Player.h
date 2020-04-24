#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "Actor.h"

class idPlayer : public idActor	
{
public:
	CLASS_PROTOTYPE(idPlayer);

	idPlayer();

	~idPlayer();
};

#endif