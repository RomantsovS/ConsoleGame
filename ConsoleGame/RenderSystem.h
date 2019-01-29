#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <memory>

#include "RenderEntity.h"

class RenderSystem
{
public:
	RenderSystem();
	
	virtual ~RenderSystem() = 0;

	virtual void addObject(std::shared_ptr<RenderEntity> object) = 0;

	virtual void init() = 0;

	virtual void update() = 0;

	virtual void clear() = 0;
};

#endif