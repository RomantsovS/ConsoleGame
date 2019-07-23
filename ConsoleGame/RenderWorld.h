#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include <list>

#include "Vector2.h"
#include "Screen.h"
#include "Model.h"

class idRenderEntityLocal;

struct renderEntity_t
{
	std::shared_ptr<idRenderModel> hModel;

	Vector2 origin;
	Vector2 axis;
};

struct renderView_t {
	// player views will set this to a non-zero integer for model suppress / allow
	// subviews (mirrors, cameras, etc) will always clear it to zero
	int viewID;
};

class idRenderWorld
{
public:
	virtual ~idRenderWorld() = 0 {}

	// The same render world can be reinitialized as often as desired
	// a NULL or empty mapName will create an empty, single area world
	virtual bool InitFromMap(const std::string mapName) = 0;

	virtual	int AddEntityDef(const renderEntity_t *re) = 0;
	virtual	void UpdateEntityDef(int entityHandle, const renderEntity_t *re) = 0;
	virtual	void FreeEntityDef(int entityHandle) = 0;
	//virtual const renderEntity_s *GetRenderEntity(int entityHandle) const = 0;

	//virtual void AddEntity(const renderEntity_s *ent) = 0;

	virtual void RenderScene(const std::shared_ptr<renderView_t> renderView) = 0;

	virtual std::shared_ptr<idRenderWorld> getptr() = 0;
};

#endif