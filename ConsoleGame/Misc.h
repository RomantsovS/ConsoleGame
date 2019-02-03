#ifndef MISC_H
#define MISC_H

#include "Entity.h"

class StaticEntity : public Entity
{
public:
	CLASS_PROTOTYPE(StaticEntity);

	StaticEntity();

	virtual ~StaticEntity() override;

	void Spawn();
	virtual void Think() override;
};

#endif