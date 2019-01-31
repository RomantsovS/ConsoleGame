#ifndef RENDER_ENTITY_BASE_H
#define RENDER_ENTITY_BASE_H

#include "RenderEntity.h"

class RenderEntityBase : public RenderEntity
{
public:
	RenderEntityBase();
	
	~RenderEntityBase() = 0;

	virtual void init() override;
	virtual void think() override;
};

#endif