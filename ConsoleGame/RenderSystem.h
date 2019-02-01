#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "RenderWorld.h"

class RenderSystem
{
public:	
	RenderSystem();

	virtual ~RenderSystem() = 0;

	virtual void init() = 0;

	virtual void draw(const renderEntity_s &ent) = 0;

	virtual void clear() = 0;
};

extern RenderSystem *renderSystem;

#endif