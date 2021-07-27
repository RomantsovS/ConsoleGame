#ifndef RENDER_WORLD_LOCAL_H
#define RENDER_WORLD_LOCAL_H

// assume any lightDef or entityDef index above this is an internal error
const int LUDICROUS_INDEX = 10000;

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
	idRenderWorldLocal(const idRenderWorldLocal&) = default;
	idRenderWorldLocal& operator=(const idRenderWorldLocal&) = default;
	idRenderWorldLocal(idRenderWorldLocal&&) = default;
	idRenderWorldLocal& operator=(idRenderWorldLocal&&) = default;

	bool InitFromMap(const std::string &mapName) override;

	int AddEntityDef(const renderEntity_t *re) override;
	void UpdateEntityDef(int entityHandle, gsl::not_null<const renderEntity_t*> re) override;
	void FreeEntityDef(int entityHandle) override;
	const renderEntity_t* GetRenderEntity(int entityHandle) const override;

	void RenderScene(const renderView_t* renderView) override;

	bool ModelTrace(modelTrace_t& trace, int entityHandle, const Vector2& start, const Vector2& end,
		const float radius) const noexcept override;

	void DebugClearLines(int time) override;

	void DrawTextToScreen(const std::string &text, const Vector2 &origin, const int color, const int lifetime) override;
	void DebugLine(const int color, const Vector2& start, const Vector2& end, const int lifetime = 0, const bool depthTest = false) noexcept override;
	void DebugBounds(const int color, const idBounds& bounds, const Vector2& org = vec2_origin, const int lifetime = 0) override;

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
	void AddWorldModelEntities() noexcept;
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

	std::shared_ptr<idRenderWorld> getptr() override;
};

#endif