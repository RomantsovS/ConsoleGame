#ifndef RENDER_ENTITY_H
#define RENDER_ENTITY_H

class RenderEntity
{
public:
	RenderEntity();

	virtual ~RenderEntity();

	virtual void init() = 0;
	virtual void update() = 0;
};

#endif