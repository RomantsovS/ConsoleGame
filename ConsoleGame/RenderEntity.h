#ifndef RENDER_ENTITY_H
#define RENDER_ENTITY_H

#include "RenderWorld.h"

class RenderEntity
{
public:
	RenderEntity();

	virtual ~RenderEntity();

	virtual void init() = 0;
	virtual void think() = 0;
private:
	renderEntity_s parms;
};

#endif