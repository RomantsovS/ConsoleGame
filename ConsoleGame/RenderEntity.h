#ifndef RENDER_ENTITY_H
#define RENDER_ENTITY_H

#include "Entity.h"

class RenderEntity
{
public:
	RenderEntity();

	virtual ~RenderEntity();

	virtual void draw() = 0 const;
};

#endif