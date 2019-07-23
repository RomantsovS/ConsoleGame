#ifndef ACTOR_H
#define ACTOR_H

#include "Entity.h"

class idActor : public idEntity
{
public:
	CLASS_PROTOTYPE(idActor);

	idActor();

	virtual ~idActor() override;

	void Spawn();
	virtual void Think() override;
};

#endif