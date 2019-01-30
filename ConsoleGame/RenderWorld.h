#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include <list>

#include "Vector2.h"
#include "RenderEntity.h"
#include "Screen.h"

struct renderEntity_s
{
	Vector2 pos;
	Screen::Pixel drawPixel;
};

class RenderWorld
{
public:
	RenderWorld();

	virtual ~RenderWorld() = 0;

	virtual void addEntity(renderEntity_s *ent) = 0;
};

class RenderEntity;

class RenderWorldLocal : public RenderWorld
{
public:
	RenderWorldLocal();

	virtual ~RenderWorldLocal();

	virtual void addEntity(renderEntity_s *ent) override;
private:
	std::list<RenderEntity*> entityes;
};

#endif