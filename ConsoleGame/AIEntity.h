#ifndef AI_ENTITY_H
#define AI_ENTITY_H

#include "EntityBase.h"

class AIEntity : public EntityBase
{
public:
	AIEntity();

	virtual ~AIEntity() = 0;

	virtual void init() = 0;
	virtual void update() override;
};

#endif