#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include <list>

#include "Vector2.h"
#include "Screen.h"
#include "RenderModel.h"

class RenderEntity;

struct renderEntity_s
{
	Vector2 pos;

	RenderModel model;
};

class RenderWorld
{
public:
	RenderWorld();

	virtual ~RenderWorld() = 0;

	virtual void AddEntity(const renderEntity_s *ent) = 0;

	virtual void RenderScene() = 0;
};

class RenderWorldLocal : public RenderWorld
{
public:
	RenderWorldLocal();

	virtual ~RenderWorldLocal();

	virtual void AddEntity(const renderEntity_s *ent) override;

	virtual void RenderScene();
private:
	std::list<RenderEntity*> entities;
};

#endif