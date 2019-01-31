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
private:
	renderEntity_s parms;
};

class RenderEntityLocal : public RenderEntity
{
public:
	RenderEntityLocal();

	~RenderEntityLocal() = 0;

	virtual void init() override;
	virtual void think() override;

	renderEntity_s parms;
};

class RenderSystemLocal : public RenderSystem
{
public:
	RenderSystemLocal(size_t ht, size_t wd, size_t bordHt, size_t bordWd, Screen::Pixel bordPix = Screen::Pixel('#', Screen::ConsoleColor::White));

	~RenderSystemLocal();

	virtual void init();

	virtual void draw();

	virtual void clear();
private:
	RenderWorld *renderWorld;

	Screen screen;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;
};

extern RenderSystemLocal tr;

#endif