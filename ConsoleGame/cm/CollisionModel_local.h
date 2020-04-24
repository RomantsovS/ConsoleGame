#include <vector>
#include <memory>

#include "CollisionModel.h"
#include "../idlib/bv/Bounds.h"

#define	MAX_SUBMODELS						2048
#define	TRACE_MODEL_HANDLE					MAX_SUBMODELS

struct cm_model_t {
	std::string					name;				// model name
	idBounds				bounds;				// model bounds
	int						contents;			// all contents of the model ored together
	bool					isConvex;			// set if model is convex
	// model geometry
	int						maxVertices;		// size of vertex array
	int						numVertices;		// number of vertices
	//cm_vertex_t* vertices;			// array with all vertices used by the model
	int						maxEdges;			// size of edge array
	int						numEdges;			// number of edges
	//cm_edge_t* edges;				// array with all edges used by the model
	//cm_node_t* node;				// first node of spatial subdivision
	// blocks with allocated memory
	/*cm_nodeBlock_t* nodeBlocks;			// list with blocks of nodes
	cm_polygonRefBlock_t* polygonRefBlocks;	// list with blocks of polygon references
	cm_brushRefBlock_t* brushRefBlocks;		// list with blocks of brush references
	cm_polygonBlock_t* polygonBlock;		// memory block with all polygons
	cm_brushBlock_t* brushBlock;			// memory block with all brushes
	*/// statistics
	int						numPolygons;
	int						polygonMemory;
	int						numBrushes;
	int						brushMemory;
	int						numNodes;
	int						numBrushRefs;
	int						numPolygonRefs;
	int						numInternalEdges;
	int						numSharpEdges;
	int						numRemovedPolys;
	int						numMergedPolys;
	int						usedMemory;
};

class idCollisionModelManagerLocal : public idCollisionModelManager {
public:
	// get clip handle for model
	int LoadModel(const std::string& modelName) override;
	// sets up a trace model for collision with other trace models
	int SetupTrmModel(const idTraceModel& trm/*, const idMaterial* material*/) override;

	// bounds of the model
	bool GetModelBounds(int model, idBounds& bounds) const override;

	// translates a trm and reports the first collision if any
	void Translation(trace_t* results, const Vector2& start, const Vector2& end,
		const std::shared_ptr<idTraceModel> trm, int contentMask, int model, const Vector2& modelOrigin) override;
	// stores all contact points of the trm with the model, returns the number of contacts
	int Contacts(std::vector<contactInfo_t>::iterator contacts, const int maxContacts, const Vector2& start,
		const Vector2& dir, const float depth, const std::shared_ptr<idTraceModel> trm, 
		int contentMask, int model, const Vector2& modelOrigin) override;
private:			// CollisionMap_load.cpp
	std::shared_ptr<cm_model_t> AllocModel();

	int FindModel(const std::string& name);
	std::shared_ptr<cm_model_t> LoadBinaryModel(const std::string& fileName);
	std::shared_ptr<cm_model_t> LoadBinaryModelFromFile();
private:
	// models
	int maxModels;
	int numModels;
	std::vector<std::shared_ptr<cm_model_t>> models;
	// for retrieving contact points
	bool			getContacts;
	std::vector<contactInfo_t>::iterator contacts;
	int				maxContacts;
	int				numContacts;
};