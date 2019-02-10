#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "RenderWorld.h"

class RenderSystem
{
public:	
	RenderSystem();

	virtual ~RenderSystem() = 0;

	virtual void Init() = 0;

	virtual void SetHeight(size_t h) = 0;
	virtual void SetWidth(size_t w) = 0;

	virtual void Draw(const renderEntity_s &ent) = 0;

	virtual void Clear() = 0;
};

extern RenderSystem *renderSystem;

#endif