#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "RenderWorld.h"

bool R_IsInitialized();

class idRenderWorld;

class idRenderSystem
{
public:	
	idRenderSystem();

	virtual ~idRenderSystem() = 0;

	virtual void Init() = 0;

	// only called before quitting
	virtual void Shutdown() = 0;

	virtual void SetHeight(size_t h) = 0;
	virtual void SetWidth(size_t w) = 0;

	// allocate a renderWorld to be used for drawing
	virtual std::shared_ptr<idRenderWorld> AllocRenderWorld() = 0;
	virtual	void FreeRenderWorld(std::shared_ptr<idRenderWorld> rw) = 0;

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	virtual void BeginLevelLoad() = 0;
	virtual void EndLevelLoad() = 0;

	virtual void Clear() = 0;
};

extern idRenderSystem *renderSystem;

#endif