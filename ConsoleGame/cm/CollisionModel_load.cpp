#include <precompiled.h>
#pragma hdrstop

#include "CollisionModel_local.h"
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
	mapFileTime = 0;
	loaded = 0;
	//checkCount = 0;
	maxModels = 0;
	numModels = 0;
	models.clear();
	//memset(trmPolygons, 0, sizeof(trmPolygons));
	trmBrushes[0] = nullptr;
	//trmMaterial = NULL;
	//numProcNodes = 0;
	//procNodes = NULL;
	getContacts = false;
	contacts = nullptr;
	maxContacts = 0;
	numContacts = 0;
}

/*
================
idCollisionModelManagerLocal::RemoveBrushReferences_r
================
*/
void idCollisionModelManagerLocal::RemoveBrushReferences_r(cm_node_t* node, const cm_brush_t* b) {
	while (node) {
		for (std::shared_ptr<cm_brushRef_t> bref = node->brushes; bref; bref = bref->next) {
			if (bref->b.get() == b) {
				bref->b = nullptr;
				return;
			}
		}
		// if leaf node
		if (node->planeType == -1) {
			break;
		}
		/*if (b->bounds[0][node->planeType] > node->planeDist) {
			node = node->children[0];
		}
		else if (b->bounds[1][node->planeType] < node->planeDist) {
			node = node->children[1];
		}
		else {
			RemoveBrushReferences_r(node->children[1], b);
			node = node->children[0];
		}*/
	}
}

/*
================
idCollisionModelManagerLocal::FreeNode
================
*/
void idCollisionModelManagerLocal::FreeNode(std::shared_ptr<cm_node_t> node) {
	// don't free the node here
	// the nodes are allocated in blocks which are freed when the model is freed
}

/*
================
idCollisionModelManagerLocal::FreeBrushReference
================
*/
void idCollisionModelManagerLocal::FreeBrushReference(std::shared_ptr<cm_brushRef_t> bref) {
	// don't free the brush reference here
	// the brush references are allocated in blocks which are freed when the model is freed
}

void idCollisionModelManagerLocal::FreeTrmModelStructure() {
	//int i;

	//assert(!models.empty());
	if (!models[MAX_SUBMODELS]) {
		return;
	}

	/*for (i = 0; i < MAX_TRACEMODEL_POLYS; i++) {
		FreePolygon(models[MAX_SUBMODELS], trmPolygons[i]->p);
	}*/
	FreeBrush(models[MAX_SUBMODELS].get(), trmBrushes[0]->b.get());
	trmBrushes[0]->b = nullptr;

	//models[MAX_SUBMODELS]->node->polygons = NULL;
	models[MAX_SUBMODELS]->node->brushes = nullptr;
	FreeModel(models[MAX_SUBMODELS]);
}

void idCollisionModelManagerLocal::FreeBrush(cm_model_t* model, cm_brush_t* brush) {
	model->numBrushes--;
	model->brushMemory -= sizeof(cm_brush_t);
	/*if (model->brushBlock == NULL) {
		Mem_Free(brush);
	}*/
}

/*
================
idCollisionModelManagerLocal::FreeTree_r
================
*/
void idCollisionModelManagerLocal::FreeTree_r(std::shared_ptr<cm_model_t> model, std::shared_ptr<cm_node_t> headNode, std::shared_ptr<cm_node_t> node) {
	// free all brushes at this node
	for (std::shared_ptr<cm_brushRef_t> bref = node->brushes; bref; bref = node->brushes) {
		std::shared_ptr<cm_brush_t> b = bref->b;
		if (b) {
			// remove all other references to this brush
			RemoveBrushReferences_r(headNode.get(), b.get());
			FreeBrush(model.get(), b.get());
		}
		node->brushes = bref->next;
		FreeBrushReference(bref);
	}
	// recurse down the tree
	if (node->planeType != -1) {
		FreeTree_r(model, headNode, node->children[0]);
		node->children[0] = NULL;
		FreeTree_r(model, headNode, node->children[1]);
		node->children[1] = NULL;
	}
	FreeNode(node);
}

void idCollisionModelManagerLocal::FreeModel(std::shared_ptr<cm_model_t> model)
{
	/*cm_polygonRefBlock_t* polygonRefBlock, * nextPolygonRefBlock;
	cm_brushRefBlock_t* brushRefBlock, * nextBrushRefBlock;
	cm_nodeBlock_t* nodeBlock, * nextNodeBlock;*/

	// free the tree structure
	if (model->node) {
		FreeTree_r(model, model->node, model->node);
	}
	// free blocks with polygon references
	/*for (polygonRefBlock = model->polygonRefBlocks; polygonRefBlock; polygonRefBlock = nextPolygonRefBlock) {
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

void idCollisionModelManagerLocal::LoadMap(const idMapFile* mapFile)
{
	if (!mapFile) {
		common->Error("idCollisionModelManagerLocal::LoadMap: NULL mapFile");
		return;
	}

	// check whether we can keep the current collision map based on the mapName and mapFileTime
	if (loaded) {
		if (mapName == mapFile->GetName()) {
			if (mapFile->GetFileTime() == mapFileTime) {
				common->DPrintf("Using loaded version\n");
				return;
			}
			common->DPrintf("Reloading modified map\n");
		}
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
	BuildModels(mapFile);

	//common->UpdateLevelLoadPacifier();

	// save name and time stamp
	mapName = mapFile->GetName();
	mapFileTime = mapFile->GetFileTime();
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
		models[numModels]->name = modelName;

		numModels++;

		/*if (cvarSystem->GetCVarBool("fs_buildresources")) {
			// for resource gathering write this model to the preload file for this map
			fileSystem->AddCollisionPreload(modelName);
		}*/
		return (numModels - 1);
	}

	return 0;
}

/*
================
idCollisionModelManagerLocal::SetupTrmModel

Trace models (item boxes, etc) are converted to collision models on the fly, using the last model slot
as a reusable temporary buffer
================
*/
int idCollisionModelManagerLocal::SetupTrmModel(const idTraceModel& trm)
{
	int i;
	const traceModelVert_t* trmVert;

	auto model = models[MAX_SUBMODELS];
	model->node->brushes = nullptr;

	// if not a valid trace model
	if (trm.type == TRM_INVALID) {
		return TRACE_MODEL_HANDLE;
	}
	// vertices
	model->numVertices = trm.numVerts;
	trmVert = trm.verts;
	for (i = 0; i < trm.numVerts; i++, trmVert++) {
		model->vertices[i].p = *trmVert;
	}
	// if the trace model is convex
	if (trm.isConvex) {
		// setup brush for position test
		trmBrushes[0]->b->bounds = trm.bounds;
		// link brush at node
		trmBrushes[0]->next = model->node->brushes;
		//trmBrushes[0]->b->material = material;
		model->node->brushes = trmBrushes[0];
	}

	// model bounds
	model->bounds = trm.bounds;
	// convex
	model->isConvex = trm.isConvex;

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

	return TrmFromModel(models[handle].get(), trm);
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

/*
===================
idCollisionModelManagerLocal::GetModelContents
===================
*/
bool idCollisionModelManagerLocal::GetModelContents(int model, int& contents) const {
	if (model < 0 || model > MAX_SUBMODELS || model >= numModels || !models[model]) {
		common->Printf("idCollisionModelManagerLocal::GetModelContents: invalid model handle\n");
		return false;
	}

	contents = models[model]->contents;

	return true;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::AllocModel() {
	auto model = std::make_shared<cm_model_t>();

	model->contents = 0;
	model->isConvex = false;
	model->maxVertices = 0;
	model->numVertices = 0;
	model->vertices.clear();
	//model->maxEdges = 0;
	//model->numEdges = 0;
	//model->edges = NULL;
	model->node = nullptr;
	/*model->nodeBlocks = NULL;
	model->polygonRefBlocks = NULL;*/
	model->brushRefBlocks = nullptr;
	/*model->polygonBlock = NULL;
	model->brushBlock = NULL;*/
	model->numBrushes = model->brushMemory =
		model->numNodes = model->numBrushRefs =
		model->usedMemory = 0;

	return model;
}

/*
================
idCollisionModelManagerLocal::AllocNode
================
*/
std::shared_ptr<cm_node_t> idCollisionModelManagerLocal::AllocNode(cm_model_t* model, int blockSize) {
	int i;
	std::shared_ptr<cm_node_t> node;
	std::shared_ptr<cm_nodeBlock_t> nodeBlock;

	if (!model->nodeBlocks || !model->nodeBlocks->nextNode) {
		nodeBlock = std::make_shared<cm_nodeBlock_t>();
		nodeBlock->nextNode = std::make_shared<cm_node_t>();
		nodeBlock->next = model->nodeBlocks;
		model->nodeBlocks = nodeBlock;
		node = nodeBlock->nextNode;
		for (i = 0; i < blockSize - 1; i++) {
			node->parent = std::make_shared<cm_node_t>();
			node = node->parent;
		}
		node->parent = nullptr;
	}

	node = model->nodeBlocks->nextNode;
	model->nodeBlocks->nextNode = node->parent;
	node->parent = nullptr;

	return node;
}

std::shared_ptr<cm_brushRef_t> idCollisionModelManagerLocal::AllocBrushReference(cm_model_t* model, int blockSize)
{
	int i;
	std::shared_ptr<cm_brushRef_t> bref;
	std::shared_ptr<cm_brushRefBlock_t> brefBlock;

	if (!model->brushRefBlocks || !model->brushRefBlocks->nextRef) {
		brefBlock = std::make_shared<cm_brushRefBlock_t>();
		brefBlock->nextRef = std::make_shared<cm_brushRef_t>();
		brefBlock->next = model->brushRefBlocks;
		model->brushRefBlocks = brefBlock;
		bref = brefBlock->nextRef;
		for (i = 0; i < blockSize - 1; i++) {
			bref->next = std::make_shared<cm_brushRef_t>();
			bref = bref->next;
		}
		bref->next = nullptr;
	}

	bref = model->brushRefBlocks->nextRef;
	model->brushRefBlocks->nextRef = bref->next;

	return bref;
}

std::shared_ptr<cm_brush_t> idCollisionModelManagerLocal::AllocBrush(cm_model_t* model)
{
	int size;

	size = sizeof(cm_brush_t);
	model->numBrushes++;
	model->brushMemory += size;
	
	auto brush = std::make_shared<cm_brush_t>();

	return brush;
}

void idCollisionModelManagerLocal::AddBrushToNode(cm_model_t* model, cm_node_t* node, std::shared_ptr<cm_brush_t> b)
{
	auto bref = AllocBrushReference(model, 1);
	bref->b = b;
	bref->next = node->brushes;
	node->brushes = bref;
	model->numBrushRefs++;
}

void idCollisionModelManagerLocal::SetupTrmModelStructure()
{
	//int i;
	std::shared_ptr<cm_node_t> node;
	std::shared_ptr<cm_model_t> model;

	// setup model
	model = AllocModel();

	//assert(models);
	models[MAX_SUBMODELS] = model;
	// create node to hold the collision data
	node = AllocNode(model.get(), 1);
	node->planeType = -1;
	model->node = node;
	// allocate vertex and edge arrays
	model->numVertices = 0;
	model->maxVertices = MAX_TRACEMODEL_VERTS;
	model->vertices.resize(model->maxVertices);

	// allocate brush for position test
	trmBrushes[0] = AllocBrushReference(model.get(), 1);
	trmBrushes[0]->b = AllocBrush(model.get());
	//trmBrushes[0]->b->primitiveNum = 0;
	trmBrushes[0]->b->bounds.Clear();
	//trmBrushes[0]->b->checkcount = 0;
	trmBrushes[0]->b->contents = -1;		// all contents
	//trmBrushes[0]->b->material = trmMaterial;
	//trmBrushes[0]->b->numPlanes = 0;
}

/*
================
idCollisionModelManagerLocal::ConvertBrushSides
================
*/
void idCollisionModelManagerLocal::ConvertBrushSides(std::shared_ptr<cm_model_t> model, const std::shared_ptr<idMapBrush> mapBrush, int primitiveNum) {
	/*int i, j;
	idMapBrushSide* mapSide;
	idFixedWinding w;
	idPlane* planes;
	const idMaterial* material;

	// fix degenerate planes
	planes = (idPlane*)_alloca16(mapBrush->GetNumSides() * sizeof(planes[0]));
	for (i = 0; i < mapBrush->GetNumSides(); i++) {
		planes[i] = mapBrush->GetSide(i)->GetPlane();
		planes[i].FixDegeneracies(DEGENERATE_DIST_EPSILON);
	}

	// create a collision polygon for each brush side
	for (i = 0; i < mapBrush->GetNumSides(); i++) {
		mapSide = mapBrush->GetSide(i);
		material = declManager->FindMaterial(mapSide->GetMaterial());
		if (!(material->GetContentFlags() & CONTENTS_REMOVE_UTIL)) {
			continue;
		}
		w.BaseForPlane(-planes[i]);
		for (j = 0; j < mapBrush->GetNumSides() && w.GetNumPoints(); j++) {
			if (i == j) {
				continue;
			}
			w.ClipInPlace(-planes[j], 0);
		}

		if (w.GetNumPoints()) {
			PolygonFromWinding(model, &w, planes[i], material, primitiveNum);
		}
	}*/
}

void idCollisionModelManagerLocal::ConvertBrush(cm_model_t* model, const idMapBrush* mapBrush, int primitiveNum) {
	int i, contents;
	idBounds bounds;

	contents = 1;
	bounds.Clear();

	/*bounds.AddPoint(vec2_origin);
	bounds.AddPoint(Vector2(gameLocal.GetHeight(), gameLocal.GetWidth()));*/

	// we are only getting the bounds for the brush so there's no need
	// to create a winding for the last brush side
	for (i = 0; i < mapBrush->GetNumSides(); i++) {
		std::shared_ptr<idMapBrushSide> mapSide = mapBrush->GetSide(i);
		for (const auto& point : mapSide->GetPoints()) {
			bounds.AddPoint(point);
		}
	}
	if (!contents) {
		return;
	}

	// create brush for position test
	auto brush = AllocBrush(model);
	//brush->checkcount = 0;
	brush->contents = contents;
	//brush->material = material;
	//brush->primitiveNum = primitiveNum;
	brush->bounds = bounds;
	/*brush->numPlanes = mapBrush->GetNumSides();
	for (i = 0; i < mapBrush->GetNumSides(); i++) {
		brush->planes[i] = planes[i];
	}*/
	AddBrushToNode(model, model->node.get(), brush);
}

/*
================
CM_CountNodeBrushes
================
*/
static int CM_CountNodeBrushes(const cm_node_t* node) {
	int count;

	count = 0;
	for (auto bref = node->brushes; bref; bref = bref->next) {
		count++;
	}
	return count;
}

/*
================
CM_R_GetModelBounds
================
*/
static void CM_R_GetNodeBounds(idBounds* bounds, std::shared_ptr<cm_node_t> node) {
	while (1) {
		for (auto bref = node->brushes; bref; bref = bref->next) {
			bounds->AddPoint(bref->b->bounds[0]);
			bounds->AddPoint(bref->b->bounds[1]);
		}
		if (node->planeType == -1) {
			break;
		}
		CM_R_GetNodeBounds(bounds, node->children[1]);
		node = node->children[0];
	}
}

/*
================
CM_GetNodeBounds
================
*/
void CM_GetNodeBounds(idBounds* bounds, std::shared_ptr<cm_node_t> node) {
	bounds->Clear();
	CM_R_GetNodeBounds(bounds, node);
	if (bounds->IsCleared()) {
		bounds->Zero();
	}
}

/*
================
CM_GetNodeContents
================
*/
int CM_GetNodeContents(std::shared_ptr<cm_node_t> node) {
	int contents;
	//cm_polygonRef_t* pref;

	contents = 0;
	while (1) {
		/*for (pref = node->polygons; pref; pref = pref->next) {
			contents |= pref->p->contents;
		}*/
		for (auto bref = node->brushes; bref; bref = bref->next) {
			contents |= bref->b->contents;
		}
		if (node->planeType == -1) {
			break;
		}
		contents |= CM_GetNodeContents(node->children[1]);
		node = node->children[0];
	}
	return contents;
}

void idCollisionModelManagerLocal::FinishModel(cm_model_t* model) {
	// remove all unused vertices and edges
	//OptimizeArrays(model);
	// get model bounds from brush and polygon bounds
	CM_GetNodeBounds(&model->bounds, model->node);

	// get model contents
	model->contents = CM_GetNodeContents(model->node);
	// total memory used by this model
	model->usedMemory = model->numVertices * sizeof(cm_vertex_t);
}

void idCollisionModelManagerLocal::BuildModels(const idMapFile* mapFile) {
	int i;

	/*idTimer timer;
	timer.Start();*/

	//if (!LoadCollisionModelFile(mapFile->GetName(), mapFile->GetGeometryCRC())) {

		if (!mapFile->GetNumEntities()) {
			return;
		}

		// load the .proc file bsp for data optimisation
		//LoadProcBSP(mapFile->GetName());

		// convert brushes and patches to collision data
		for (i = 0; i < mapFile->GetNumEntities(); i++) {
			const std::shared_ptr<idMapEntity> mapEnt = mapFile->GetEntity(i);

			if (numModels >= MAX_SUBMODELS) {
				common->Error("idCollisionModelManagerLocal::BuildModels: more than %d collision models", MAX_SUBMODELS);
				break;
			}
			models[numModels] = CollisionModelForMapEntity(mapEnt);
			if (models[numModels]) {
				numModels++;
			}
		}

		// free the proc bsp which is only used for data optimization
		/*Mem_Free(procNodes);
		procNodes = NULL;

		// write the collision models to a file
		WriteCollisionModelsToFile(mapFile->GetName(), 0, numModels, mapFile->GetGeometryCRC());*/
	//}

	//timer.Stop();

	// print statistics on collision data
	cm_model_t model;
	AccumulateModelInfo(&model);
	common->Printf("collision data:\n");
	common->Printf("%6i models\n", numModels);
	PrintModelInfo(&model);
	//common->Printf("%.0f msec to load collision data.\n", timer.Milliseconds());
}

int idCollisionModelManagerLocal::FindModel(const std::string& name) {
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
==================
idCollisionModelManagerLocal::PrintModelInfo
==================
*/
void idCollisionModelManagerLocal::PrintModelInfo(const cm_model_t* model) {
	common->Printf("%6i vertices (%i KB)\n", model->numVertices, (model->numVertices * sizeof(cm_vertex_t)) >> 10);
	//common->Printf("%6i edges (%i KB)\n", model->numEdges, (model->numEdges * sizeof(cm_edge_t)) >> 10);
	//common->Printf("%6i polygons (%i KB)\n", model->numPolygons, model->polygonMemory >> 10);
	common->Printf("%6i brushes (%i KB)\n", model->numBrushes, model->brushMemory >> 10);
	common->Printf("%6i nodes (%i KB)\n", model->numNodes, (model->numNodes * sizeof(cm_node_t)) >> 10);
	//common->Printf("%6i polygon refs (%i KB)\n", model->numPolygonRefs, (model->numPolygonRefs * sizeof(cm_polygonRef_t)) >> 10);
	common->Printf("%6i brush refs (%i KB)\n", model->numBrushRefs, (model->numBrushRefs * sizeof(cm_brushRef_t)) >> 10);
	//common->Printf("%6i internal edges\n", model->numInternalEdges);
	//common->Printf("%6i sharp edges\n", model->numSharpEdges);
	//common->Printf("%6i contained polygons removed\n", model->numRemovedPolys);
	//common->Printf("%6i polygons merged\n", model->numMergedPolys);
	common->Printf("%6i KB total memory used\n", model->usedMemory >> 10);
}

/*
================
idCollisionModelManagerLocal::AccumulateModelInfo
================
*/
void idCollisionModelManagerLocal::AccumulateModelInfo(cm_model_t* model) {
	int i;

	// accumulate statistics of all loaded models
	for (i = 0; i < numModels; i++) {
		model->numVertices += models[i]->numVertices;
		//model->numEdges += models[i]->numEdges;
		//model->numPolygons += models[i]->numPolygons;
		//model->polygonMemory += models[i]->polygonMemory;
		model->numBrushes += models[i]->numBrushes;
		model->brushMemory += models[i]->brushMemory;
		model->numNodes += models[i]->numNodes;
		model->numBrushRefs += models[i]->numBrushRefs;
		//model->numPolygonRefs += models[i]->numPolygonRefs;
		//model->numInternalEdges += models[i]->numInternalEdges;
		//model->numSharpEdges += models[i]->numSharpEdges;
		//model->numRemovedPolys += models[i]->numRemovedPolys;
		//model->numMergedPolys += models[i]->numMergedPolys;
		model->usedMemory += models[i]->usedMemory;
	}
}

/*
=================
CM_EstimateVertsAndEdges
=================
*/
static void CM_EstimateVertsAndEdges(const idMapEntity* mapEnt, int* numVerts/*, int* numEdges*/) {
	int j;

	*numVerts = /**numEdges =*/ 0;
	for (j = 0; j < mapEnt->GetNumPrimitives(); j++) {
		const std::shared_ptr<idMapPrimitive> mapPrim = mapEnt->GetPrimitive(j);
		/*if (mapPrim->GetType() == idMapPrimitive::TYPE_PATCH) {
			// assume maximum tesselation without adding verts
			width = static_cast<const idMapPatch*>(mapPrim)->GetWidth();
			height = static_cast<const idMapPatch*>(mapPrim)->GetHeight();
			*numVerts += width * height;
			*numEdges += (width - 1) * height + width * (height - 1) + (width - 1) * (height - 1);
			continue;
		}*/
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			// assume cylinder with a polygon with (numSides - 2) edges ontop and on the bottom
			*numVerts += (std::dynamic_pointer_cast<idMapBrush>(mapPrim)->GetNumSides() - 2) * 2;
			//*numEdges += (static_cast<const idMapBrush*>(mapPrim)->GetNumSides() - 2) * 3;
			continue;
		}
	}
	* numVerts = 4;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::CollisionModelForMapEntity(const std::shared_ptr<idMapEntity> mapEnt) {
	std::shared_ptr<cm_model_t> model;
	idBounds bounds;
	std::string name;
	int i{}, brushCount{};

	// if the entity has no primitives
	if (mapEnt->GetNumPrimitives() < 1) {
		return nullptr;
	}

	// get a name for the collision model
	mapEnt->epairs.GetString("model", "", &name);
	if (name.empty()) {
		mapEnt->epairs.GetString("name", "", &name);
		if (name.empty()) {
			if (!numModels) {
				// first model is always the world
				name = "worldMap";
			}
			else {
				name = "unnamed inline model";
			}
		}
	}

	model = AllocModel();
	model->node = AllocNode(model.get(), NODE_BLOCK_SIZE_SMALL);

	CM_EstimateVertsAndEdges(mapEnt.get(), &model->maxVertices/*, &model->maxEdges*/);
	model->numVertices = 0;
	//model->numEdges = 0;
	model->vertices.resize(model->maxVertices);
	//model->edges = (cm_edge_t*)Mem_ClearedAlloc(model->maxEdges * sizeof(cm_edge_t), TAG_COLLISION);

	//cm_vertexHash->ResizeIndex(model->maxVertices);
	//cm_edgeHash->ResizeIndex(model->maxEdges);

	model->name = name;
	model->isConvex = false;

	// convert brushes
	for (i = 0; i < mapEnt->GetNumPrimitives(); i++) {
		std::shared_ptr<idMapPrimitive> mapPrim = mapEnt->GetPrimitive(i);
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			ConvertBrush(model.get(), dynamic_cast<idMapBrush*>(mapPrim.get()), 1);
			continue;
		}
	}

	// create an axial bsp tree for the model if it has more than just a bunch brushes
	brushCount = CM_CountNodeBrushes(model->node.get());
	/*if (brushCount > 4) {
		model->node = CreateAxialBSPTree(model, model->node);
	}
	else {*/
		model->node->planeType = -1;
	//}

	// get bounds for hash
	if (brushCount) {
		CM_GetNodeBounds(&bounds, model->node);
	}
	else {
		bounds[0].Set(0, 0);
		bounds[1].Set(50, 50);
	}

	// different models do not share edges and vertices with each other, so clear the hash
	//ClearHash(bounds);

	// create polygons from patches and brushes
	for (i = 0; i < mapEnt->GetNumPrimitives(); i++) {
		std::shared_ptr<idMapPrimitive> mapPrim = mapEnt->GetPrimitive(i);
		/*if (mapPrim->GetType() == idMapPrimitive::TYPE_PATCH) {
			ConvertPatch(model, static_cast<idMapPatch*>(mapPrim), i);
			continue;
		}*/
		if (mapPrim->GetType() == idMapPrimitive::TYPE_BRUSH) {
			ConvertBrushSides(model, std::dynamic_pointer_cast<idMapBrush>(mapPrim), i);
			continue;
		}
	}

	FinishModel(model.get());

	return model;
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::LoadBinaryModel(const std::string& fileName) {
	/*idFileLocal file(fileSystem->OpenFileReadMemory(fileName));
	if (file == NULL) {
		return NULL;
	}*/
	return LoadBinaryModelFromFile();
}

std::shared_ptr<cm_model_t> idCollisionModelManagerLocal::LoadBinaryModelFromFile() {
	auto model = AllocModel();
	model->name = "def cm model name";

	model->bounds.Zero();
	model->bounds.AddPoint(vec2_point_size);
	model->isConvex = true;
	model->numVertices = 1;
	model->numBrushes = 1;
	model->numNodes = 1;
	model->numBrushRefs = 1;
	model->maxVertices = model->numVertices;

	model->vertices.resize(model->numVertices);
	model->vertices[0].p = vec2_origin;

	model->brushMemory = 1;

	std::vector<std::shared_ptr<cm_brush_t>> brushes;
	brushes.resize(model->numBrushes);

	for (int i = 0; i != brushes.size(); ++i) {
		brushes[i] = AllocBrush(model.get());
		brushes[i]->bounds.Zero();
		brushes[i]->bounds.AddPoint(vec2_point_size);
		brushes[i]->contents = -1;
	}

	struct local {
		static void ReadNodeTree(cm_model_t* model, cm_node_t* node, std::vector<std::shared_ptr<cm_brush_t>>& brushes) {
			node->planeType = -1;
			int i = 0;
			
			std::shared_ptr<cm_brushRef_t> bref = collisionModelManagerLocal.AllocBrushReference(model, model->numBrushRefs);
			bref->b = brushes[i];
			bref->next = node->brushes;
			node->brushes = bref;

			/*if (node->planeType != -1) {
				node->children[0] = collisionModelManagerLocal.AllocNode(model, model->numNodes);
				node->children[1] = collisionModelManagerLocal.AllocNode(model, model->numNodes);
				node->children[0]->parent = node;
				node->children[1]->parent = node;
				ReadNodeTree(file, model, node->children[0], polys, brushes);
				ReadNodeTree(file, model, node->children[1], polys, brushes);
			}*/
		}
	};
	model->node = AllocNode(model.get(), model->numNodes + 1);
	local::ReadNodeTree(model.get(), model->node.get(), brushes);

	model->usedMemory = model->numVertices * sizeof(Vector2) +
		//model->numEdges * sizeof(cm_edge_t) +
		//model->polygonMemory +
		model->brushMemory +
		model->numNodes * sizeof(cm_node_t) +
		//model->numPolygonRefs * sizeof(cm_polygonRef_t) +
		model->numBrushRefs * sizeof(cm_brushRef_t);
	return model;
}

bool idCollisionModelManagerLocal::TrmFromModel(const cm_model_t* model, idTraceModel& trm) {
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
	trm.bounds.Zero();

	// copy vertices
	for (i = 0; i < model->numVertices; i++) {
		trm.verts[i] = model->vertices[i].p;
		trm.bounds.AddPoint(trm.verts[i] + vec2_point_size);
	}
	trm.numVerts = model->numVertices;

	// assume convex
	trm.isConvex = true;

	return true;
}