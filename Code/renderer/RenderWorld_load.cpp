#pragma hdrstop
#include "idlib/precompiled.h"

#include "tr_local.h"

/*
================
idRenderWorldLocal::FreeWorld
================
*/
void idRenderWorldLocal::FreeWorld() {
	// this will free all the lightDefs and entityDefs
	FreeDefs();

	// free all the portals and check light/model references
	for (int i = 0; i < numPortalAreas; i++) {
		portalArea_t *area;

		area = &portalAreas[i];

		// there shouldn't be any remaining lightRefs or entityRefs
		if (area->entityRefs->areaNext != area->entityRefs) {
			common->Error("FreeWorld: unexpected remaining entityRefs");
		}
		area->entityRefs->areaNext = nullptr;
		area->entityRefs->areaPrev = nullptr;
		area->entityRefs = nullptr;
	}

	if (!portalAreas.empty())
	{
		portalAreas.clear();
		numPortalAreas = 0;
	}

	if (!areaNodes.empty())
	{
		areaNodes.clear();
	}

	// free all the inline idRenderModels 
	/*for (int i = 0; i < localModels.Num(); i++) {
		renderModelManager->RemoveModel(localModels[i]);
		delete localModels[i];
	}
	localModels.Clear();*/

	mapName = "<FREED>";
}

/*
================ =
idRenderWorldLocal::InitFromMap

A NULL or empty name will make a world without a map model, which
is still useful for displaying a bare model
================ =
*/
bool idRenderWorldLocal::InitFromMap(const std::string &name) {
	// if this is an empty world, initialize manually
	if (name.empty()) {
		FreeWorld();
		mapName.clear();
		ClearWorld();
		return true;
	}

	if (name == mapName) {
		//if (fileSystem->InProductionMode() || (currentTimeStamp != FILE_NOT_FOUND_TIMESTAMP && currentTimeStamp == mapTimeStamp)) {
			//common->Printf("idRenderWorldLocal::InitFromMap: retaining existing map\n");
			FreeDefs();
			//TouchWorldModels();
			AddWorldModelEntities();
			//ClearPortalStates();
			return true;
		//}
		common->Printf("idRenderWorldLocal::InitFromMap: timestamp has changed, reloading.\n");
	}

	FreeWorld();

	mapName = name;

	ReadBinaryAreaPortals();
	ReadBinaryNodes();

	// if it was a trivial map without any areas, create a single area
	if (!numPortalAreas) {
		ClearWorld();
	}

	AddWorldModelEntities();

	return true;
}

/*
================
idRenderWorldLocal::SetupAreaRefs
================
*/
void idRenderWorldLocal::SetupAreaRefs() {
	for (int i = 0; i < numPortalAreas; i++) {
		portalAreas[i].areaNum = i;

		portalAreas[i].entityRefs = std::make_shared<areaReference_t>();
		portalAreas[i].entityRefs->areaNext = portalAreas[i].entityRefs->areaPrev = portalAreas[i].entityRefs;
	}
}

void idRenderWorldLocal::AddWorldModelEntities() noexcept {
	// add the world model for each portal area
	// we can't just call AddEntityDef, because that would place the references
	// based on the bounding box, rather than explicitly into the correct area
	/*for (int i = 0; i < numPortalAreas; i++)
	{
		auto def = std::make_shared<idRenderEntityLocal>();

		entityDefs.push_back(def);

		def->world = getptr();
	}*/
}

void idRenderWorldLocal::ReadBinaryAreaPortals() {
	numPortalAreas = 1;

	portalAreas.resize(numPortalAreas);

	// set the doubly linked lists
	SetupAreaRefs();
}

void idRenderWorldLocal::ReadBinaryNodes() {
	numAreaNodes = 1;

	areaNodes.resize(numAreaNodes);

	for (int i = 0; i < numAreaNodes; i++)
	{
		auto node = &areaNodes[i];

		node->children[0] = -1;
		node->children[1] = -1;
	}
}

/*
=================
idRenderWorldLocal::ClearWorld

Sets up for a single area world
=================
*/
void idRenderWorldLocal::ClearWorld() {
	numPortalAreas = 1;
	portalAreas.clear();
	portalAreas.resize(1);

	SetupAreaRefs();

	// even though we only have a single area, create a node
	// that has both children pointing at it so we don't need to
	//
	areaNodes.clear();
	areaNodes.resize(1);
	areaNodes[0].children[0] = -1;
	areaNodes[0].children[1] = -1;
}

/*
=================
idRenderWorldLocal::FreeDefs

dump all the interactions
=================
*/
void idRenderWorldLocal::FreeDefs() {
	// free all entityDefs
	for (size_t i = 0; i < entityDefs.size(); i++) {
		auto mod = entityDefs[i];
		if (mod && mod->world == this)
		{
			FreeEntityDef(i);
			entityDefs[i] = nullptr;
		}
	}

	//entityDefs.clear();
}