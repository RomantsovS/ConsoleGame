#ifndef PLAYER_ENTITY_H
#define PLAYER_ENTITY_H

#include "Actor.h"

class Player : public Actor	
{
public:
	CLASS_PROTOTYPE(idPlayer);

	Player();

	~Player();
};

#endif