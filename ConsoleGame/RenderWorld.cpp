#include "tr_local.h"
#include "RenderWorld_local.h"

idRenderWorldLocal::idRenderWorldLocal()
{
	mapName.clear();

	areaNodes.clear();
	numAreaNodes = 0;

	portalAreas.clear();
	numPortalAreas = 0;
}

idRenderWorldLocal::~idRenderWorldLocal()
{
	// free all the entityDefs, lightDefs, portals, etc
	FreeWorld();
}

int idRenderWorldLocal::AddEntityDef(const renderEntity_t * re)
{
	// try and reuse a free spot
	int entityHandle;

	auto iter = std::find(entityDefs.begin(), entityDefs.end(), nullptr);

	if (iter == entityDefs.end())
	{
		entityHandle = entityDefs.size();
		entityDefs.resize(entityDefs.size() + 4);
	}
	else
		entityHandle = iter - entityDefs.begin();

	UpdateEntityDef(entityHandle, re);

	return entityHandle;
}

void idRenderWorldLocal::UpdateEntityDef(int entityHandle, const renderEntity_t * re)
{
	while (entityHandle >= static_cast<int>(entityDefs.size()))
	{
		entityDefs.resize(entityDefs.size() + 4);
	}

	std::shared_ptr<idRenderEntityLocal> def = entityDefs[entityHandle];

	if (def)
	{
		// save any decals if the model is the same, allowing marks to move with entities
		if (def->parms.hModel == re->hModel)
		{
			R_FreeEntityDefDerivedData(def, true, true);
		}
		else {
			R_FreeEntityDefDerivedData(def, false, false);
		}
	}
	else
	{
		// creating a new one
		def = std::make_shared<idRenderEntityLocal>();
		entityDefs[entityHandle] = def;

		def->world = std::dynamic_pointer_cast<idRenderWorldLocal>(getptr());
		def->index = entityHandle;
	}

	def->parms = *re;

	// based on the model bounds, add references in each area
	// that may contain the updated surface
	R_CreateEntityRefs(def);
}

/*
===================
FreeEntityDef

Frees all references and lit surfaces from the model, and
NULL's out it's entry in the world list
===================
*/
void idRenderWorldLocal::FreeEntityDef(int entityHandle)
{
	if (entityHandle < 0 || entityHandle >= static_cast<int>(entityDefs.size()))
	{
		std::out_of_range("idRenderWorld::FreeEntityDef: handle %i " + std::to_string(entityHandle) + "> %i" + std::to_string(entityDefs.size()) + "\n");
		return;
	}

	auto def = entityDefs[entityHandle];
	if (!def)
	{
		return;
	}

	R_FreeEntityDefDerivedData(def, false, false);

	//R_FreeEntityDefDerivedData(def, false, false);

	// if we are playing a demo, these will have been freed
	// in R_FreeEntityDefDerivedData(), otherwise the gui
	// object still exists in the game

	entityDefs[entityHandle] = nullptr;
}

void idRenderWorldLocal::RenderScene(const std::shared_ptr<renderView_t> renderView)
{
	tr.FillBorder();

	auto parms = std::make_shared<viewDef_t>();
	parms->renderWorld = std::dynamic_pointer_cast<idRenderWorldLocal>(getptr());

	R_RenderView(parms);

	tr.Display();
}

void idRenderWorldLocal::AddEntityRefToArea(std::shared_ptr<idRenderEntityLocal> def, portalArea_t* area)
{
	if (!def)
	{
		return;
	}

	for (auto ref = def->entityRefs; ref; ref = ref->ownerNext) {
		if (ref->area == area) {
			return;
		}
	}

	auto ref = std::make_shared<areaReference_t>();

	ref->entity = def;

	// link to entityDef
	ref->ownerNext = def->entityRefs;
	def->entityRefs = ref;

	// link to end of area list
	ref->area = area;
	ref->areaNext = area->entityRefs;
	ref->areaPrev = area->entityRefs->areaPrev;
	ref->areaNext->areaPrev = ref;
	ref->areaPrev->areaNext = ref;
}

void idRenderWorldLocal::PushFrustumIntoTree_r(std::shared_ptr<idRenderEntityLocal> def, int nodeNum)
{
	if (nodeNum < 0) {
		int areaNum = -1 - nodeNum;
		auto area = &portalAreas[areaNum];
		if (area->viewCount == tr.viewCount) {
			return;	// already added a reference here
		}
		area->viewCount = tr.viewCount;

		if (def)
		{
			AddEntityRefToArea(def, area);
		}

		return;
	}

	auto node = &areaNodes.at(nodeNum);

	nodeNum = node->children[0];
	if (nodeNum != 0) {	// 0 = solid
		PushFrustumIntoTree_r(def, nodeNum);
	}

	nodeNum = node->children[1];
	if (nodeNum != 0) {	// 0 = solid
		PushFrustumIntoTree_r(def, nodeNum);
	}
}

void idRenderWorldLocal::PushFrustumIntoTree(std::shared_ptr<idRenderEntityLocal> def)
{
	if (areaNodes.empty())
	{
		return;
	}

	PushFrustumIntoTree_r(def, 0);
}

std::shared_ptr<idRenderWorld> idRenderWorldLocal::getptr()
{
	return shared_from_this();
}
