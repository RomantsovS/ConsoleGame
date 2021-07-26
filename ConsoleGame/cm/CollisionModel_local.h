#include "CollisionModel.h"

#define	MAX_SUBMODELS						2048
#define	TRACE_MODEL_HANDLE					MAX_SUBMODELS

#define NODE_BLOCK_SIZE_SMALL				1//8

/*
===============================================================================

Collision model

===============================================================================
*/

struct cm_vertex_t {
	Vector2					p;					// vertex point
	//int						checkcount;			// for multi-check avoidance
	//unsigned long			side;				// each bit tells at which side this vertex passes one of the trace model edges
	//unsigned long			sideSet;			// each bit tells if sidedness for the trace model edge has been calculated yet
};

struct cm_brush_t {
	cm_brush_t() {
		//checkcount = 0;
		contents = 0;
		//material = NULL;
		//primitiveNum = 0;
		//numPlanes = 0;
	}
	//int						checkcount;			// for multi-check avoidance
	idBounds				bounds;				// brush bounds
	int						contents;			// contents of brush
	//const idMaterial* material;			// material
	//int						primitiveNum;		// number of brush primitive
	//int						numPlanes;			// number of bounding planes
	//idPlane					planes[1];			// variable sized
};

struct cm_brushRef_t {
	std::shared_ptr<cm_brush_t> b;					// pointer to brush
	std::shared_ptr<cm_brushRef_t> next;			// next brush in chain
};

struct cm_brushRefBlock_t {
	std::shared_ptr<cm_brushRef_t> nextRef;			// next brush reference in block
	std::shared_ptr<cm_brushRefBlock_t> next;			// next block with brush references
};

struct cm_node_t {
	int						planeType;			// node axial plane type
	//float					planeDist;			// node plane distance
	//std::shared_ptr<cm_polygonRef_t> polygons;			// polygons in node
	std::shared_ptr<cm_brushRef_t> brushes;		// brushes in node
	std::shared_ptr<cm_node_t> parent;			// parent of this node
	std::shared_ptr<cm_node_t> children[2];		// node children
};

struct cm_nodeBlock_t {
	std::shared_ptr<cm_node_t> nextNode;			// next node in block
	std::shared_ptr<cm_nodeBlock_t> next;			// next block with nodes
};

struct cm_model_t {
	cm_model_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "cm_model_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~cm_model_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		if(isCommonExists)
			common->DPrintf("%s dtor\n", "cm_model_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	cm_model_t(const cm_model_t&) = default;
	cm_model_t& operator=(const cm_model_t&) = default;
	cm_model_t(cm_model_t&&) = default;
	cm_model_t& operator=(cm_model_t&&) = default;

	std::string					name{};				// model name
	idBounds				bounds{};				// model bounds
	int						contents{};			// all contents of the model ored together
	bool					isConvex{};			// set if model is convex
	// model geometry
	int						maxVertices{};		// size of vertex array
	int						numVertices{};		// number of vertices
	std::vector<cm_vertex_t> vertices{};			// array with all vertices used by the model
	//int						maxEdges;			// size of edge array
	//int						numEdges;			// number of edges
	//cm_edge_t* edges;				// array with all edges used by the model
	std::shared_ptr<cm_node_t> node{};				// first node of spatial subdivision
	// blocks with allocated memory
	std::shared_ptr<cm_nodeBlock_t> nodeBlocks{};			// list with blocks of nodes
	//cm_polygonRefBlock_t* polygonRefBlocks;	// list with blocks of polygon references
	std::shared_ptr<cm_brushRefBlock_t> brushRefBlocks{};		// list with blocks of brush references
	//cm_polygonBlock_t* polygonBlock;		// memory block with all polygons
	//std::shared_ptr<cm_brushBlock_t> brushBlock;			// memory block with all brushes
	// statistics
	/*int						numPolygons;
	int						polygonMemory;*/
	int						numBrushes{};
	int						brushMemory{};
	int						numNodes{};
	int						numBrushRefs{};
	/*int						numPolygonRefs;
	int						numInternalEdges;
	int						numSharpEdges;
	int						numRemovedPolys;
	int						numMergedPolys;*/
	int						usedMemory{};
};

/*
===============================================================================

Data used during collision detection calculations

===============================================================================
*/

struct cm_trmVertex_t {
	int used;										// true if this vertex is used for collision detection
	Vector2 p;										// vertex position
	Vector2 endp;									// end point of vertex after movement
	int polygonSide;								// side of polygon this vertex is on (rotational collision)
	//idPluecker pl;									// pluecker coordinate for vertex movement
	//idVec3 rotationOrigin;							// rotation origin for this vertex
	idBounds rotationBounds;						// rotation bounds for this vertex
};

struct cm_traceWork_t {
	cm_traceWork_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "cm_traceWork_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~cm_traceWork_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		if(isCommonExists)
			common->DPrintf("%s ctor\n", "cm_traceWork_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	cm_traceWork_t(const cm_traceWork_t&) = default;
	cm_traceWork_t& operator=(const cm_traceWork_t&) = default;
	cm_traceWork_t(cm_traceWork_t&&) = default;
	cm_traceWork_t& operator=(cm_traceWork_t&&) = default;

	int numVerts;
	cm_trmVertex_t vertices[MAX_TRACEMODEL_VERTS];	// trm vertices
	//int numEdges;
	//cm_trmEdge_t edges[MAX_TRACEMODEL_EDGES + 1];		// trm edges
	//int numPolys;
	//cm_trmPolygon_t polys[MAX_TRACEMODEL_POLYS];	// trm polygons
	std::weak_ptr<cm_model_t> model;				// model colliding with
	Vector2 start;									// start of trace
	Vector2 end;									// end of trace
	Vector2 dir;									// trace direction
	idBounds bounds;								// bounds of full trace
	idBounds size;									// bounds of transformed trm relative to start
	Vector2 extents;									// largest of abs(size[0]) and abs(size[1]) for BSP trace
	int contents;									// ignore polygons that do not have any of these contents flags
	trace_t trace;									// collision detection result

	//bool rotation;									// true if calculating rotational collision
	bool pointTrace;								// true if only tracing a point
	bool positionTest;								// true if not tracing but doing a position test
	bool isConvex;									// true if the trace model is convex
	//bool axisIntersectsTrm;							// true if the rotation axis intersects the trace model
	bool getContacts;								// true if retrieving contacts
	bool quickExit;									// set to quickly stop the collision detection calculations

	Vector2 origin;									// origin of rotation in model space
	//idVec3 axis;									// rotation axis in model space
	//idMat3 matrix;									// rotates axis of rotation to the z-axis
	//float angle;									// angle for rotational collision
	//float maxTan;									// max tangent of half the positive angle used instead of fraction
	//float radius;									// rotation radius of trm start
	//idRotation modelVertexRotation;					// inverse rotation for model vertices

	contactInfo_t* contacts;						// array with contacts
	int maxContacts;								// max size of contact array
	int numContacts;								// number of contacts found

	//idPlane heartPlane1;							// polygons should be near anough the trace heart planes
	//float maxDistFromHeartPlane1;
	//idPlane heartPlane2;
	//float maxDistFromHeartPlane2;
	//idPluecker polygonEdgePlueckerCache[CM_MAX_POLYGON_EDGES];
	//idPluecker polygonVertexPlueckerCache[CM_MAX_POLYGON_EDGES];
	//idVec3 polygonRotationOriginCache[CM_MAX_POLYGON_EDGES];
};

class idCollisionModelManagerLocal : public idCollisionModelManager {
public:
	// load collision models from a map file
	void LoadMap(const idMapFile* mapFile) override;
	// frees all the collision models
	void FreeMap() override;

	//void Preload(const char* mapName);
	// get clip handle for model
	int LoadModel(const std::string& modelName) override;
	// sets up a trace model for collision with other trace models
	int SetupTrmModel(const idTraceModel& trm/*, const idMaterial* material*/) override;
	// Creates a trace model from a collision model, returns true if succesfull.
	bool TrmFromModel(const std::string& modelName, idTraceModel& trm) override;

	// bounds of the model
	bool GetModelBounds(int model, idBounds& bounds) const override;
	// all contents flags of brushes and polygons ored together
	bool GetModelContents(int model, int& contents) const override;
	// translates a trm and reports the first collision if any
	void Translation(trace_t* results, const Vector2& start, const Vector2& end,
		const idTraceModel* trm, int contentMask, int model, const Vector2& modelOrigin) override;
	// stores all contact points of the trm with the model, returns the number of contacts
	int Contacts(contactInfo_t* contacts, const int maxContacts, const Vector2& start,
		const Vector2& dir, const float depth, const idTraceModel* trm, 
		int contentMask, int model, const Vector2& modelOrigin) override;

private:			// CollisionMap_translate.cpp
	void SetupTrm(cm_traceWork_t* tw, const idTraceModel* trm);
	void TranslationIter(trace_t* results, const Vector2& start, const Vector2& end,
		const idTraceModel* trm, int contentMask, int model, const Vector2& modelOrigin);
private:			// CollisionMap_contents.cpp
	bool TestTrmVertsInBrush(cm_traceWork_t* tw, cm_brush_t* b);
	int ContentsTrm(trace_t* results, const Vector2& start,
		const idTraceModel* trm, int contentMask,
		int model, const Vector2& modelOrigin);
private:			// CollisionMap_trace.cpp
	void TraceTrmThroughNode(cm_traceWork_t* tw, cm_node_t* node);
	void TraceThroughAxialBSPTree_r(cm_traceWork_t* tw, cm_node_t* node, float p1f, float p2f, Vector2& p1, Vector2& p2);
	void TraceThroughModel(cm_traceWork_t* tw);
private:			// CollisionMap_load.cpp
	void Clear();
	void FreeTrmModelStructure();
	// model deallocation
	void RemoveBrushReferences_r(cm_node_t* node, const cm_brush_t* b);
	void FreeNode(std::shared_ptr<cm_node_t> node);
	void FreeBrushReference(std::shared_ptr<cm_brushRef_t> bref);

	// model deallocation
	void FreeBrush(cm_model_t* model, cm_brush_t* brush);
	void FreeTree_r(std::shared_ptr<cm_model_t> model, std::shared_ptr<cm_node_t> headNode, std::shared_ptr<cm_node_t> node);
	void FreeModel(std::shared_ptr<cm_model_t> model);

	// creation of axial BSP tree
	std::shared_ptr<cm_model_t> AllocModel();
	std::shared_ptr<cm_node_t> AllocNode(cm_model_t* model, int blockSize);
	std::shared_ptr<cm_brushRef_t> AllocBrushReference(cm_model_t* model, int blockSize);
	std::shared_ptr<cm_brush_t> AllocBrush(cm_model_t* model);
	void AddBrushToNode(cm_model_t* model, cm_node_t* node, std::shared_ptr<cm_brush_t> b);
	void SetupTrmModelStructure();
	
	// creation of raw polygons
	void ConvertBrushSides(std::shared_ptr<cm_model_t> model, const std::shared_ptr<idMapBrush> mapBrush, int primitiveNum);
	void ConvertBrush(cm_model_t* model, gsl::not_null<const idMapBrush*> mapBrush, int primitiveNum);
	void			PrintModelInfo(const cm_model_t* model);
	void			AccumulateModelInfo(cm_model_t* model);
	void FinishModel(cm_model_t* model);
	void BuildModels(gsl::not_null<const idMapFile*> mapFile);
	int FindModel(const std::string& name);
	std::shared_ptr<cm_model_t> CollisionModelForMapEntity(const std::shared_ptr<idMapEntity> mapEnt);	// brush/patch model from .map
	std::shared_ptr<cm_model_t> LoadBinaryModel(const std::string& fileName);
	std::shared_ptr<cm_model_t> LoadBinaryModelFromFile();
	bool TrmFromModel(const cm_model_t* model, idTraceModel& trm);
private:			// collision map data
	std::string mapName;
	long long mapFileTime;
	int loaded;
	// models
	int maxModels;
	int numModels;
	std::vector<std::shared_ptr<cm_model_t>> models;
	// polygons and brush for trm model
	std::shared_ptr<cm_brushRef_t> trmBrushes[1];
	// for retrieving contact points
	bool			getContacts;
	contactInfo_t* contacts;
	int				maxContacts;
	int				numContacts;
};

void CM_AddContact(gsl::not_null<cm_traceWork_t*> tw);