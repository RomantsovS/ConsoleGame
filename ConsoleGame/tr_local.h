#ifndef RENDER_ENTITY_H
#define RENDER_ENTITY_H

#include "RenderWorld.h"
#include "RenderSystem.h"

class RenderEntity
{
public:
	RenderEntity();

	virtual ~RenderEntity();

	virtual void init() = 0;
	virtual void think() = 0;

	renderEntity_s parms;
};

class RenderEntityLocal : public RenderEntity
{
public:
	RenderEntityLocal();

	~RenderEntityLocal();

	virtual void init() override;
	virtual void think() override;
};

class RenderSystemLocal : public RenderSystem
{
public:
	RenderSystemLocal();

	~RenderSystemLocal();

	virtual void init();

	virtual void draw(const renderEntity_s &ent);

	virtual void clear();

	void fillBorder();
private:
	RenderWorld *renderWorld;

	Screen screen;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;
};

extern RenderSystemLocal tr;

#endif