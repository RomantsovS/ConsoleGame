#ifndef ENTITY_H
#define ENTITY_H

#include "RenderEntity.h"
#include "Physics.h"

class Entity
{
public:
	Entity();
	
	virtual ~Entity() = 0;

	virtual void init() = 0;
	virtual void update() = 0;
};

#endif