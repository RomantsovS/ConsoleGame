#ifndef RENDER_WORLD_LOCAL_H
#define RENDER_WORLD_LOCAL_H

#include <list>
#include <string>

#include "Vector2.h"
#include "Screen.h"
#include "Model.h"
#include "tr_local.h"

struct areaReference_t;

struct portalArea_t
{
	int				areaNum;
	int				viewCount;		// set by R_FindViewLightsAndEntities
	std::shared_ptr<areaReference_t> entityRefs;		// head/tail of doubly linked list, may change
};

struct areaNode_t
{
	int				children[2];		// negative numbers are (-1 - areaNumber), 0 = solid
	int				commonChildrenArea;	// if all children are either solid or a single area,
										// this is the area number, else CHILDREN_HAVE_MULTIPLE_AREAS
};

class idRenderWorldLocal : public idRenderWorld, public std::enable_shared_from_this<idRenderWorldLocal>
{
public:
	idRenderWorldLocal();

	virtual ~idRenderWorldLocal();

	virtual	bool InitFromMap(const std::string &mapName) override;

	virtual	int AddEntityDef(const renderEntity_t *re) override;
	virtual	void UpdateEntityDef(int entityHandle, const renderEntity_t *re) override;
	virtual	void FreeEntityDef(int entityHandle) override;
	//virtual const renderEntity_s *GetRenderEntity(int entityHandle) const = 0;

	virtual void RenderScene(const std::shared_ptr<renderView_t> renderView);

	std::string mapName; // ie: maps/tim_dm2.proc, written to demoFile

	std::vector<areaNode_t> areaNodes;
	int numAreaNodes;

	std::vector<portalArea_t> portalAreas;
	int numPortalAreas;

	std::vector<std::shared_ptr<idRenderEntityLocal>> entityDefs;

	//-----------------------
	// RenderWorld_load.cpp
	void SetupAreaRefs();
	void FreeWorld();
	void ClearWorld();
	void FreeDefs();
	void AddWorldModelEntities();
	void ReadBinaryAreaPortals();
	void ReadBinaryNodes();

	//--------------------------
	// RenderWorld_portals.cpp
	void AddAreaViewEntities(int areaNum);
	void AddAreaToView(int areaNum);
	void FindViewLightsAndEntities();

	//--------------------------
	// RenderWorld.cpp
	void AddEntityRefToArea(std::shared_ptr<idRenderEntityLocal> def, portalArea_t *area);

	void PushFrustumIntoTree_r(std::shared_ptr<idRenderEntityLocal> def, int nodeNum);
	void PushFrustumIntoTree(std::shared_ptr<idRenderEntityLocal> def);

	virtual std::shared_ptr<idRenderWorld> getptr();
};

#endif