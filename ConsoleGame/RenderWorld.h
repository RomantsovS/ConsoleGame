#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include <list>

#include "Vector2.h"
#include "RenderEntity.h"
#include "Screen.h"

struct renderEntity_s
{
	Vector2 pos;
};

class RenderWorld
{
public:
	RenderWorld();

	virtual ~RenderWorld() = 0;

	virtual void addEntity(const renderEntity_s *ent) = 0;

	virtual void renderScene() = 0;
};

class RenderWorldLocal : public RenderWorld
{
public:
	RenderWorldLocal();

	virtual ~RenderWorldLocal();

	virtual void addEntity(const renderEntity_s *ent) override;

	virtual void renderScene();
private:
	std::list<RenderEntity*> entityDefs;
};

#endif