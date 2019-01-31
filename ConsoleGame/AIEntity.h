#ifndef AI_ENTITY_H
#define AI_ENTITY_H

#include "Entity.h"

class AIEntity : public Entity
{
public:
	AIEntity();

	virtual ~AIEntity() override;

	virtual void init() override;
	virtual void think() override;
};

#endif