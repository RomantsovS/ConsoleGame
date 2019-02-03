#ifndef ACTOR_H
#define ACTOR_H

#include "Entity.h"

class Actor : public Entity
{
public:
	CLASS_PROTOTYPE(Actor);

	Actor();

	virtual ~Actor() override;

	void Spawn();
	virtual void Think() override;
};

#endif