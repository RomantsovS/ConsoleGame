#ifndef AI_H
#define AI_H

#include "Actor.h"

class AI : public Actor
{
public:
	CLASS_PROTOTYPE(AI);

	AI();

	virtual ~AI() override;

	void Spawn();
	virtual void Think() override;
};

#endif