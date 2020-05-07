#include <stdexcept>

#include "CollisionModel_local.h"
#include "../framework/Common.h"
#include "../idlib/sys/sys_assert.h"
#include "../d3xp/Game_local.h"

idCollisionModelManagerLocal collisionModelManagerLocal;
idCollisionModelManager* collisionModelManager = &collisionModelManagerLocal;

/*
===============================================================================

Free map

===============================================================================
*/

/*
================
idCollisionModelManagerLocal::Clear
================
*/
void idCollisionModelManagerLocal::Clear() {
	mapName.clear();
	//mapFileTime = 0;
	loaded = 0;
	//checkCount = 0;
	maxModels = 0;
	numModels = 0;
	models.clear();
	//memset(trmPolygons, 0, sizeof(trmPolygons));
	//trmBrushes[0] = NULL;
	//trmMaterial = NULL;
	//numProcNodes = 0;
	//procNodes = NULL;
	getContacts = false;
	//contacts = nullptr;
	maxContacts = 0;
	numContacts = 0;
}

void idCollisionModelManagerLocal::FreeTrmModelStructure()
{
	//int i;

	//assert(!models.empty());
	if (!models[MAX_SUBMODELS]) {
		return;
	}

	/*for (i = 0; i < MAX_TRACEMODEL_POLYS; i++) {
		FreePolygon(models[MAX_SUBMODELS], trmPolygons[i]->p);
	}
	FreeBrush(models[MAX_SUBMODELS], trmBrushes[0]->b);

	models[MAX_SUBMODELS]->node->polygons = NULL;
	models[MAX_SUBMODELS]->node->brushes = NULL;*/
	FreeModel(models[MAX_SUBMODELS]);
}

void idCollisionModelManagerLocal::FreeModel(std::shared_ptr<cm_model_t> model)
{
	/*cm_polygonRefBlock_t* polygonRefBlock, * nextPolygonRefBlock;
	cm_brushRefBlock_t* brushRefBlock, * nextBrushRefBlock;
	cm_nodeBlock_t* nodeBlock, * nextNodeBlock;

	// free the tree structure
	if (model->node) {
		FreeTree_r(model, model->node, model->node);
	}
	// free blocks with polygon references
	for (polygonRefBlock = model->polygonRefBlocks; polygonRefBlock; polygonRefBlock = nextPolygonRefBlock) {
		nextPolygonRefBlock = polygonRefBlock->next;
		Mem_Free(polygonRefBlock);
	}
	// free blocks with brush references
	for (brushRefBlock = model->brushRefBlocks; brushRefBlock; brushRefBlock = nextBrushRefBlock) {
		nextBrushRefBlock = brushRefBlock->next;
		Mem_Free(brushRefBlock);
	}
	// free blocks with nodes
	for (nodeBlock = model->nodeBlocks; nodeBlock; nodeBlock = nextNodeBlock) {
		nextNodeBlock = nodeBlock->next;
		Mem_Free(nodeBlock);
	}
	// free block allocated polygons
	Mem_Free(model->polygonBlock);
	// free block allocated brushes
	Mem_Free(model->brushBlock);
	// free edges
	Mem_Free(model->edges);*/
	// free vertices
	model->vertices.clear();
	// free the model
	model = nullptr;
}

void idCollisionModelManagerLocal::LoadMap(/*const idMapFile* mapFile*/)
{
	/*if (mapFile == NULL) {
		common->Error("idCollisionModelManagerLocal::LoadMap: NULL mapFile");
		return;
	}*/

	// check whether we can keep the current collision map based on the mapName and mapFileTime
	if (loaded) {
		/*if (mapName.Icmp(mapFile->GetName()) == 0) {
			if (mapFile->GetFileTime() == mapFileTime) {
				common->DPrintf("Using loaded version\n");
				return;
			}
			common->DPrintf("Reloading modified map\n");
		}*/
		FreeMap();
	}

	// clear the collision map
	Clear();

	// models
	maxModels = MAX_SUBMODELS;
	numModels = 0;
	models.resize(maxModels + 1);

	// setup hash to speed up finding shared vertices and edges
	//SetupHash();

	//common->UpdateLevelLoadPacifier();

	// setup trace model structure
	SetupTrmModelStructure();

	//common->UpdateLevelLoadPacifier();

	// build collision models
	BuildModels(/*mapFile*/);

	//common->UpdateLevelLoadPacifier();

	// save name and time stamp
	//mapName = mapFile->GetName();
	//mapFileTime = mapFile->GetFileTime();
	loaded = true;

	// shutdown the hash
	//ShutdownHash();
}

void idCollisionModelManagerLocal::FreeMap()
{
	int i;

	if (!loaded) {
		Clear();
		return;
	}

	for (i = 0; i < maxModels; i++) {
		if (!models[i]) {
			continue;
		}
		FreeModel(models[i]);
	}

	FreeTrmModelStructure();

	models.clear();

	Clear();

	//ShutdownHash();
}

/*
==================
idCollisionModelManagerLocal::LoadModel
==================
*/
int idCollisionModelManagerLocal::LoadModel(const std::string& modelName) {
	int handle;

	handle = FindModel(modelName);
	if (handle >= 0) {
		return handle;
	}

	if (numModels >= MAX_SUBMODELS) {
		common->Error("idCollisionModelManagerLocal::LoadModel: no free slots\n");
		return 0;
	}

	std::string generatedFileName = "generated/collision/";
	//generatedFileName.AppendPath(modelName);
	//generatedFileName.SetFileExtension(CMODEL_BINARYFILE_EXT);

	//ID_TIME_T sourceTimeStamp = fileSystem->GetTimestamp(modelName);
	//int sourceTimeStamp = 0;

	models[numModels] = LoadBinaryModel(generatedFileName);
	if (models[numModels]) {
		numModels++;
		/*if (cvarSystem->GetCVarBool("fs_buildresources")) {
			// for resource gathering write this model to the preload file for this map
			fileSystem->AddCollisionPreload(modelName);
		}*/
		return (numModels - 1);
	}

	return 0;
}

int idCollisionModelManagerLocal::SetupTrmModel(const idTraceModel& trm)
{
	auto model = models[MAX_SUBMODELS];
	// if not a valid trace model
	if (trm.type == TRM_INVALID) {
		return TRACE_MODEL_HANDLE;
	}
	// vertices
	model->numVertices = trm.numVerts;
	// model bounds
	model->bounds = trm.bounds;

	return TRACE_MODEL_HANDLE;
}

bool idCollisionModelManagerLocal::TrmFromModel(const std::string& modelName, idTraceModel& trm)
{
	int handle;

	handle = LoadModel(modelName);
	if (!handle) {
		common->Printf("idCollisionModelManagerLocal::TrmFromModel: model %s not found.\n", modelName);
		return false;
	}

	return TrmFromModel(models[handle], trm);
}

bool idCollisionModelManagerLocal::GetModelBounds(int model, idBounds& bounds) const
{
	if (model < 0 || model > MAX_SUBMODELS || model >= numModels || !models[model]) {
		common->Printf("idCollisionModelManagerLocal::GetModelBounds: invalid model handle\n");
		return false;
	}

	bounds = models[model]->bounds;
	return true;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::AllocModel()
{
	auto model = std::make_shared<cm_model_t>();
	model->contents = 0;
	model->isConvex = false;
	model->maxVertices = 0;
	model->numVertices = 0;
	model->vertices.clear();
	//model->maxEdges = 0;
	//model->numEdges = 0;
	/*model->edges = NULL;
	model->node = NULL;
	model->nodeBlocks = NULL;
	model->polygonRefBlocks = NULL;
	model->brushRefBlocks = NULL;
	model->polygonBlock = NULL;
	model->brushBlock = NULL;*/
	model->usedMemory = 0;

	return model;
}

void idCollisionModelManagerLocal::SetupTrmModelStructure()
{
	//int i;
	//cm_node_t* node;
	std::shared_ptr<cm_model_t> model;

	// setup model
	model = AllocModel();

	//assert(models);
	models[MAX_SUBMODELS] = model;
	// create node to hold the collision data
	/*node = (cm_node_t*)AllocNode(model, 1);
	node->planeType = -1;
	model->node = node;*/
	// allocate vertex and edge arrays
	model->numVertices = 0;
	model->maxVertices = MAX_TRACEMODEL_VERTS;
	model->vertices.resize(model->maxVertices);
}

void idCollisionModelManagerLocal::FinishModel(std::shared_ptr<cm_model_t> model)
{
	// remove all unused vertices and edges
	//OptimizeArrays(model);
	// get model bounds from brush and polygon bounds
	//CM_GetNodeBounds(&model->bounds, model->node);
	model->bounds.Clear();
	model->bounds.AddPoint(vec2_origin);
	model->bounds.AddPoint(Vector2(static_cast<float>(gameLocal.GetHeight() - 1), static_cast<float>(gameLocal.GetWidth() - 1)));

	// get model contents
	//model->contents = CM_GetNodeContents(model->node);
	// total memory used by this model
	model->usedMemory = model->numVertices * sizeof(cm_vertex_t);
}

void idCollisionModelManagerLocal::BuildModels()
{
	//int i;
	//const idMapEntity* mapEnt;

	/*idTimer timer;
	timer.Start();*/

	/*if (!LoadCollisionModelFile(mapFile->GetName(), mapFile->GetGeometryCRC())) {

		if (!mapFile->GetNumEntities()) {
			return;
		}*/

		// load the .proc file bsp for data optimisation
		//LoadProcBSP(mapFile->GetName());

		// convert brushes and patches to collision data
		//for (i = 0; i < mapFile->GetNumEntities(); i++) {
			//mapEnt = mapFile->GetEntity(i);

			if (numModels >= MAX_SUBMODELS) {
				common->Error("idCollisionModelManagerLocal::BuildModels: more than %d collision models", MAX_SUBMODELS);
				//break;
			}
			models[numModels] = CollisionModelForMapEntity(/*mapEnt*/);
			if (models[numModels]) {
				numModels++;
			}
		//}

		// free the proc bsp which is only used for data optimization
		/*Mem_Free(procNodes);
		procNodes = NULL;

		// write the collision models to a file
		WriteCollisionModelsToFile(mapFile->GetName(), 0, numModels, mapFile->GetGeometryCRC());*/
	//}

	//timer.Stop();

	// print statistics on collision data
	/*cm_model_t model;
	AccumulateModelInfo(&model);
	common->Printf("collision data:\n");
	common->Printf("%6i models\n", numModels);
	PrintModelInfo(&model);
	common->Printf("%.0f msec to load collision data.\n", timer.Milliseconds());*/
}

int idCollisionModelManagerLocal::FindModel(const std::string& name)
{
	int i;

	// check if this model is already loaded
	for (i = 0; i < numModels; i++) {
		if (models[i]->name == name) {
			break;
		}
	}
	// if the model is already loaded
	if (i < numModels) {
		return i;
	}
	return -1;
}

/*
=================
CM_EstimateVertsAndEdges
=================
*/
static void CM_EstimateVertsAndEdges(/*const idMapEntity* mapEnt,*/ int* numVerts/*, int* numEdges*/) {
	//int j, width, height;

	*numVerts = /**numEdges =*/ 0;
	/*for (j = 0; j < mapEnt->GetNumPrimitives(); j++) {
		const idMapPrimitive* mapPrim;
		mapPrim = mapEnt->GetPrimitive(j);
		if (mapPrim->GetType() == idMapPrimitive::TYPE_PATCH) {
			// assume maximum tesselation without adding verts
			width = static_cast<const idMapPatch*>(mapPrim)->GetWidth();
			height = static_cast<const idMapPatch*>(mapPrim)->GetHeight();
			*numVerts += width * height;
			*numEdges += (width - 1) * height + width * (height - 1) + (width - 1) * (height - 1);
			continue;
		}
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			// assume cylinder with a polygon with (numSides - 2) edges ontop and on the bottom
			*numVerts += (static_cast<const idMapBrush*>(mapPrim)->GetNumSides() - 2) * 2;
			*numEdges += (static_cast<const idMapBrush*>(mapPrim)->GetNumSides() - 2) * 3;
			continue;
		}
	}*/
	* numVerts = 4;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::CollisionModelForMapEntity()
{
	std::shared_ptr<cm_model_t> model;
	idBounds bounds;
	std::string name;
	//int i , brushCount;

	// if the entity has no primitives
	/*if (mapEnt->GetNumPrimitives() < 1) {
		return NULL;
	}

	// get a name for the collision model
	/*mapEnt->epairs.GetString("model", "", &name);
	if (!name[0]) {
		mapEnt->epairs.GetString("name", "", &name);
		if (!name[0]) {*/
			if (!numModels) {
				// first model is always the world
				name = "worldMap";
			}
			else {
				name = "unnamed inline model";
			}
		//}
	//}

	model = AllocModel();
	//model->node = AllocNode(model, NODE_BLOCK_SIZE_SMALL);

	CM_EstimateVertsAndEdges(/*mapEnt,*/ &model->maxVertices/*, &model->maxEdges*/);
	model->numVertices = 0;
	//model->numEdges = 0;
	model->vertices.resize(model->maxVertices);
	//model->edges = (cm_edge_t*)Mem_ClearedAlloc(model->maxEdges * sizeof(cm_edge_t), TAG_COLLISION);

	//cm_vertexHash->ResizeIndex(model->maxVertices);
	//cm_edgeHash->ResizeIndex(model->maxEdges);

	model->name = name;
	model->isConvex = false;

	// convert brushes
	/*for (i = 0; i < mapEnt->GetNumPrimitives(); i++) {
		idMapPrimitive* mapPrim;

		mapPrim = mapEnt->GetPrimitive(i);
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			ConvertBrush(model, static_cast<idMapBrush*>(mapPrim), i);
			continue;
		}
	}

	// create an axial bsp tree for the model if it has more than just a bunch brushes
	brushCount = CM_CountNodeBrushes(model->node);
	if (brushCount > 4) {
		model->node = CreateAxialBSPTree(model, model->node);
	}
	else {
		model->node->planeType = -1;
	}*/

	// get bounds for hash
	/*if (brushCount) {
		CM_GetNodeBounds(&bounds, model->node);
	}
	else {
		bounds[0].Set(0, 0);
		bounds[1].Set(50, 50);
	}*/

	// different models do not share edges and vertices with each other, so clear the hash
	//ClearHash(bounds);

	// create polygons from patches and brushes
	/*for (i = 0; i < mapEnt->GetNumPrimitives(); i++) {
		idMapPrimitive* mapPrim;

		mapPrim = mapEnt->GetPrimitive(i);
		if (mapPrim->GetType() == idMapPrimitive::TYPE_PATCH) {
			ConvertPatch(model, static_cast<idMapPatch*>(mapPrim), i);
			continue;
		}
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			ConvertBrushSides(model, static_cast<idMapBrush*>(mapPrim), i);
			continue;
		}
	}*/

	FinishModel(model);

	return model;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::LoadBinaryModel(const std::string& fileName)
{
	/*idFileLocal file(fileSystem->OpenFileReadMemory(fileName));
	if (file == NULL) {
		return NULL;
	}*/
	return LoadBinaryModelFromFile();
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::LoadBinaryModelFromFile()
{
	auto model = AllocModel();
	model->name = "def cm model name";

	model->bounds = idBounds(Vector2(0, 0));
	model->numVertices = 1;

	model->maxVertices = model->numVertices;

	model->vertices.resize(1);
	for(size_t i = 0; i != 1; ++i)
		model->vertices[i].p = Vector2(0, 0);

	model->usedMemory = model->numVertices * sizeof(Vector2);
		//model->numEdges * sizeof(cm_edge_t) +
		//model->polygonMemory +
		//model->brushMemory +
		//model->numNodes * sizeof(cm_node_t) +
		//model->numPolygonRefs * sizeof(cm_polygonRef_t) +
		//model->numBrushRefs * sizeof(cm_brushRef_t);
	return model;
}

bool idCollisionModelManagerLocal::TrmFromModel(const std::shared_ptr<cm_model_t> model, idTraceModel& trm)
{
	int i;// , j, numEdgeUsers[MAX_TRACEMODEL_EDGES + 1];

	// if the model has too many vertices to fit in a trace model
	if (model->numVertices > MAX_TRACEMODEL_VERTS) {
		common->Printf("idCollisionModelManagerLocal::TrmFromModel: model %s has too many vertices.\n", model->name.c_str());
		//PrintModelInfo(model);
		return false;
	}

	// plus one because the collision model accounts for the first unused edge
	/*if (model->numEdges > MAX_TRACEMODEL_EDGES + 1) {
		common->Printf("idCollisionModelManagerLocal::TrmFromModel: model %s has too many edges.\n", model->name.c_str());
		//PrintModelInfo(model);
		return false;
	}*/

	trm.type = TRM_CUSTOM;
	trm.numVerts = 0;
	//trm.numEdges = 1;
	//trm.numPolys = 0;
	trm.bounds.Clear();

	// copy vertices
	for (i = 0; i < model->numVertices; i++) {
		trm.verts[i] = model->vertices[i].p;
		trm.bounds.AddPoint(trm.verts[i]);
	}
	trm.numVerts = model->numVertices;

	return true;
}