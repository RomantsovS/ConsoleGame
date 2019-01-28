#ifndef ENTITY_H
#define ENTITY_H

#include "RenderEntity.h"

class Entity
{
public:
	Entity();
	
	virtual ~Entity();

	virtual RenderEntity *getRenderEntity() = 0 const;
};

#endif