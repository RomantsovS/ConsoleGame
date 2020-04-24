#ifndef AI_H
#define AI_H

#include "../Actor.h"

class idAI : public idActor
{
public:
	CLASS_PROTOTYPE(idAI);

	idAI();

	virtual ~idAI() override;

	void Spawn();
	virtual void Think() override;
};

#endif