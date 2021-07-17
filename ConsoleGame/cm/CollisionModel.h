#ifndef COLLISION_MODEL_H
#define COLLISION_MODEL_H

// contact type
typedef enum {
	CONTACT_NONE,							// no contact
	CONTACT_EDGE,							// trace model edge hits model edge
	CONTACT_MODELVERTEX,					// model vertex hits trace model polygon
	CONTACT_TRMVERTEX						// trace model vertex hits model polygon
} contactType_t;

// contact info
struct contactInfo_t {
	contactType_t			type;			// contact type
	Vector2					point;			// point of contact
	//Vector2					normal;			// contact plane normal
	//float					dist;			// contact plane distance
	int						contents;		// contents at other side of surface
	//const idMaterial* material;		// surface material
	int						modelFeature;	// contact feature on model
	int						trmFeature;		// contact feature on trace model
	int						entityNum;		// entity the contact surface is a part of
	int						id;				// id of clip model the contact surface is part of
};

// trace result
struct trace_t {
	float					fraction;		// fraction of movement completed, 1.0 = didn't hit anything
	Vector2					endpos;			// final position of trace model
	//idMat3					endAxis;		// final axis of trace model
	contactInfo_t			c;				// contact information, only valid if fraction < 1.0
};

extern const float CM_CLIP_EPSILON;
extern const float CM_BOX_EPSILON;
extern const float CM_POINT_SIZE;
//extern const float CM_POINT_BOX;
extern const float CM_MAX_TRACE_DIST;
extern Vector2 vec3_pointEpsilon;
extern Vector2 vec2_point_size;

class idCollisionModelManager {
public:
	virtual ~idCollisionModelManager() {}

	// Loads collision models from a map file.
	virtual void			LoadMap(const idMapFile* mapFile) = 0;
	// Frees all the collision models.
	virtual void			FreeMap() = 0;

	// Gets the clip handle for a model.
	virtual int LoadModel(const std::string& modelName) = 0;
	// Sets up a trace model for collision with other trace models.
	virtual int SetupTrmModel(const idTraceModel& trm/*, const idMaterial* material*/) = 0;
	// Creates a trace model from a collision model, returns true if succesfull.
	virtual bool TrmFromModel(const std::string& modelName, idTraceModel& trm) = 0;

	// Gets the bounds of a model.
	virtual bool GetModelBounds(int model, idBounds& bounds) const = 0;
	// Gets all contents flags of brushes and polygons of a model ored together.
	virtual bool GetModelContents(int model, int& contents) const = 0;
	// translates a trm and reports the first collision if any
	virtual void Translation(trace_t* results, const Vector2& start, const Vector2& end,
		const idTraceModel* trm, int contentMask, int model, const Vector2& modelOrigin) = 0;
	// stores all contact points of the trm with the model, returns the number of contacts
	virtual int Contacts(contactInfo_t* contacts, const int maxContacts, const Vector2& start,
		const Vector2& dir, const float depth, const idTraceModel* trm,
		int contentMask, int model, const Vector2& modelOrigin) = 0;
};

extern idCollisionModelManager* collisionModelManager;

#endif // ! COLLISION_MODEL_H
