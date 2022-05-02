#ifndef CLIP_H
#define CLIP_H

#define JOINT_HANDLE_TO_CLIPMODEL_ID( id )	( -1 - id )

class idClip;
class idClipModel;
class idEntity;

struct clipLink_t;
struct clipSector_t;

class idClipModel : public std::enable_shared_from_this<idClipModel>{
	friend class idClip;
public:
	idClipModel();
	explicit idClipModel(const std::string& name);
	explicit idClipModel(const idTraceModel& trm);
	explicit idClipModel(const idClipModel& model);
	~idClipModel();
	idClipModel& operator=(const idClipModel&) = default;
	idClipModel(idClipModel&&) = default;
	idClipModel& operator=(idClipModel&&) = default;

	bool LoadModel(const std::string&  name);
	void LoadModel(const idTraceModel& trm, bool persistantThroughSave = true);

	void Link(idClip& clp);				// must have been linked with an entity and id before
	void Link(idClip& clp, idEntity* ent, int newId, const Vector2& newOrigin, int renderModelHandle = -1);
	void Unlink() noexcept; // unlink from sectors
	//void					SetPosition(const idVec3& newOrigin, const idMat3& newAxis);	// unlinks the clip model
	void Translate(const Vector2& translation) noexcept;							// unlinks the clip model
	//void					Rotate(const idRotation& rotation);							// unlinks the clip model
	void Enable() noexcept;						// enable for clipping
	void Disable() noexcept;					// keep linked but disable for clipping
	void SetEntity(idEntity* newEntity) noexcept;
	idEntity* GetEntity() const noexcept;
	void SetId(int newId) noexcept;
	int GetId() const noexcept;
	void SetOwner(idEntity* newOwner) noexcept;
	idEntity* GetOwner() const noexcept;
	const idBounds& GetBounds() const noexcept;
	const idBounds& GetAbsBounds() const noexcept;
	const Vector2& GetOrigin() const noexcept;
	bool IsTraceModel() const noexcept; // returns true if this is a trace model
	int Handle() const; // returns handle used to collide vs this model

	static int CheckModel(const std::string& name);
	static void ClearTraceModelCache() noexcept;
private:
	bool enabled; // true if this clip model is used for clipping
	idEntity* entity; // entity using this clip model
	int id; // id for entities that use multiple clip models
	idEntity* owner; // owner of the entity that owns this clip model
	Vector2 origin; // origin of clip model
	//idMat3 axis; // orientation of clip model
	idBounds bounds; // bounds
	idBounds absBounds; // absolute bounds
	//const idMaterial* material; // material for trace models
	int contents; // all contents ored together
	int collisionModelHandle;	// handle to collision model
	int traceModelIndex; // trace model used for collision detection
	int renderModelHandle; // render model def handle

	std::shared_ptr<clipLink_t> clipLinks; // links into sectors
	int touchCount;

	void Init(); // initialize
	void Link_r(clipSector_t* node);

	static int AllocTraceModel(const idTraceModel& trm, bool persistantThroughSaves = true);
	static void FreeTraceModel(int traceModelIndex);
	static idTraceModel* GetCachedTraceModel(int traceModelIndex);
	static int GetTraceModelHashKey(const idTraceModel& trm) noexcept;
};

inline void idClipModel::SetEntity(idEntity* newEntity) noexcept {
	entity = newEntity;
}

inline void idClipModel::SetOwner(idEntity* newOwner) noexcept {
	owner = newOwner;
}

inline idEntity* idClipModel::GetEntity() const noexcept {
	return entity;
}

inline idEntity* idClipModel::GetOwner() const noexcept {
	return owner;
}

inline const idBounds& idClipModel::GetBounds() const noexcept {
	return bounds;
}

inline const idBounds& idClipModel::GetAbsBounds() const noexcept {
	return absBounds;
}

inline const Vector2& idClipModel::GetOrigin() const noexcept {
	return origin;
}

inline bool idClipModel::IsTraceModel() const noexcept {
	return (traceModelIndex != -1);
}

inline void idClipModel::Translate(const Vector2& translation) noexcept {
	Unlink();
	origin += translation;
}

inline void idClipModel::Enable() noexcept {
	enabled = true;
}

inline void idClipModel::Disable() noexcept {
	enabled = false;
}

inline void idClipModel::SetId(int newId) noexcept {
	id = newId;
}

inline int idClipModel::GetId() const noexcept {
	return id;
}

/*
====================
idClip::ClipModelsTouchingBounds_r
====================
*/
struct listParms_t {
	idBounds		bounds;
	int				contentMask;
	idClipModel** list;
	int				count;
	int				maxCount;
};

//===============================================================
//
//	idClip
//
//===============================================================

class idClip {

	friend class idClipModel;

public:
	idClip();
	~idClip();
	idClip(const idClip&) = default;
	idClip& operator=(const idClip&) = default;
	idClip(idClip&&) = default;
	idClip& operator=(idClip&&) = default;

	void Init();
	void Shutdown();

	// clip versus the rest of the world
	bool Translation(trace_t& results, const Vector2& start, const Vector2& end,
		const idClipModel* mdl, int contentMask, const idEntity* passEntity);
	bool Motion(trace_t& results, const Vector2& start, const Vector2& end,
		const idClipModel* mdl, int contentMask, const idEntity* passEntity);

	int Contacts(std::vector<contactInfo_t>& contacts, const int maxContacts, const Vector2& start,
		const Vector2& dir, const float depth, const idClipModel* mdl, int contentMask,
		const idEntity* passEntity);

	int ClipModelsTouchingBounds(const idBounds& bounds, int contentMask, idClipModel** clipModelList,
		int maxCount) const;

	const idBounds& GetWorldBounds() const noexcept;
	std::shared_ptr<idClipModel> DefaultClipModel() noexcept;

	// stats and debug drawing
	void PrintStatistics() noexcept;
	void DrawClipModels(const Vector2& eye, const float radius, const idEntity* passEntity);
	void DrawClipSectors();
private:
	int						numClipSectors;
	std::vector<std::shared_ptr<clipSector_t>> clipSectors;
	idBounds worldBounds;
	idClipModel temporaryClipModel;
	std::shared_ptr<idClipModel> defaultClipModel;
	mutable int				touchCount;
	// statistics
	int						numTranslations;
	int						numRotations;
	int						numMotions;
	int						numRenderModelTraces;
	int						numContents;
	int						numContacts;
private:
	std::shared_ptr<clipSector_t> CreateClipSectors_r(const int depth, const idBounds& bounds, Vector2& maxSector);
	void ClipModelsTouchingBounds_r(const clipSector_t* node, listParms_t& parms) const;
	const idTraceModel* TraceModelForClipModel(const idClipModel* mdl) const;
	int GetTraceClipModels(const idBounds& bounds, int contentMask,
		const idEntity* passEntity, idClipModel** clipModelList) const;
	void TraceRenderModel(trace_t& trace, const Vector2& start, const Vector2& end, const float radius,
		idClipModel* touch) const;

	bool DrawClipSectors_r(const clipSector_t* node, const idBounds& bounds);
};

inline const idBounds& idClip::GetWorldBounds() const noexcept {
	return worldBounds;
}

inline std::shared_ptr<idClipModel> idClip::DefaultClipModel() noexcept {
	return defaultClipModel;
}

#endif // ! CLIP_H
