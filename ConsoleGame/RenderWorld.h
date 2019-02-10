#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H

#include <list>

#include "Vector2.h"
#include "Screen.h"
#include "Model.h"

class RenderEntityLocal;

struct renderEntity_s
{
	RenderModel *hModel;

	Vector2 origin;
	Vector2 axis;
};

class RenderWorld
{
public:
	RenderWorld();

	virtual ~RenderWorld() = 0;

	virtual	int AddEntityDef(const renderEntity_s *re) = 0;
	virtual	void UpdateEntityDef(int entityHandle, const renderEntity_s *re) = 0;
	virtual	void FreeEntityDef(int entityHandle) = 0;
	//virtual const renderEntity_s *GetRenderEntity(int entityHandle) const = 0;

	//virtual void AddEntity(const renderEntity_s *ent) = 0;

	virtual void RenderScene() = 0;
};

class RenderWorldLocal : public RenderWorld
{
public:
	RenderWorldLocal();

	virtual ~RenderWorldLocal();

	//virtual void AddEntity(const renderEntity_s *ent) override;

	virtual	int AddEntityDef(const renderEntity_s *re) override;
	virtual	void UpdateEntityDef(int entityHandle, const renderEntity_s *re) override;
	virtual	void FreeEntityDef(int entityHandle) override;
	//virtual const renderEntity_s *GetRenderEntity(int entityHandle) const = 0;

	virtual void RenderScene();
private:
	std::vector<RenderEntityLocal*> entityDefs;
};

#endif