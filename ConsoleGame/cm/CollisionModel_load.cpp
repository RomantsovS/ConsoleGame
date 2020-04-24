#include <stdexcept>

#include "CollisionModel_local.h"

idCollisionModelManagerLocal collisionModelManagerLocal;
idCollisionModelManager* collisionModelManager = &collisionModelManagerLocal;

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
		//common->Error("idCollisionModelManagerLocal::LoadModel: no free slots\n");
		return 0;
	}

	std::string generatedFileName = "generated/collision/";
	//generatedFileName.AppendPath(modelName);
	//generatedFileName.SetFileExtension(CMODEL_BINARYFILE_EXT);

	//ID_TIME_T sourceTimeStamp = fileSystem->GetTimestamp(modelName);
	int sourceTimeStamp = 0;

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

bool idCollisionModelManagerLocal::GetModelBounds(int model, idBounds& bounds) const
{
	if (model < 0 || model > MAX_SUBMODELS || model >= numModels || !models[model]) {
		throw std::logic_error("idCollisionModelManagerLocal::GetModelBounds: invalid model handle\n");
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
	//model->vertices = NULL;
	model->maxEdges = 0;
	model->numEdges = 0;
	/*model->edges = NULL;
	model->node = NULL;
	model->nodeBlocks = NULL;
	model->polygonRefBlocks = NULL;
	model->brushRefBlocks = NULL;
	model->polygonBlock = NULL;
	model->brushBlock = NULL;*/
	model->numPolygons = model->polygonMemory =
		model->numBrushes = model->brushMemory =
		model->numNodes = model->numBrushRefs =
		model->numPolygonRefs = model->numInternalEdges =
		model->numSharpEdges = model->numRemovedPolys =
		model->numMergedPolys = model->usedMemory = 0;

	return model;
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
	
	model->bounds = idBounds(Vector2(1, 1));
	model->numVertices = 1;

	model->maxVertices = model->numVertices;

	model->usedMemory = model->numVertices * sizeof(Vector2);
		//model->numEdges * sizeof(cm_edge_t) +
		//model->polygonMemory +
		//model->brushMemory +
		//model->numNodes * sizeof(cm_node_t) +
		//model->numPolygonRefs * sizeof(cm_polygonRef_t) +
		//model->numBrushRefs * sizeof(cm_brushRef_t);
	return model;
}
