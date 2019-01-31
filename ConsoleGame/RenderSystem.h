#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "RenderWorld.h"

class RenderSystem
{
public:	
	virtual ~RenderSystem() = 0;

	virtual void init() = 0;

	virtual void draw() = 0;

	virtual void clear() = 0;
};

extern RenderSystem *renderSystem;

#endif