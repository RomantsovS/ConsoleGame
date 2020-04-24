#ifndef CLIP_H
#define CLIP_H

#include <memory>
#include <vector>

#include "../../idlib/math/Vector2.h"
#include "../../idlib/bv/Bounds.h"
#include "../../idlib/geometry/TraceModel.h"
#include "../../cm/CollisionModel.h"

#define JOINT_HANDLE_TO_CLIPMODEL_ID( id )	( -1 - id )

class idCilp;
class idClipModel;
class idEntity;

struct clipLink_t;

struct clipSector_t {
	int axis;		// -1 = leaf node
	float dist;
	std::shared_ptr<clipSector_t> children[2];
	std::weak_ptr<clipLink_t> clipLinks;
};

struct clipLink_t {
	std::weak_ptr<idClipModel> clipModel;
	std::shared_ptr<clipSector_t> sector;
	std::weak_ptr<clipLink_t> prevInSector;
	std::shared_ptr<clipLink_t> nextInSector;
	std::shared_ptr<clipLink_t> nextLink;
};

struct trmCache_t {
	trmCache_t() : trm(std::make_shared<idTraceModel>()) {}

	std::shared_ptr<idTraceModel> trm;
	int						refCount;
	//float					volume;
	//Vector2					centerOfMass;
};

class idClipModel : private std::enable_shared_from_this<idClipModel>{
	friend class idClip;
public:
	idClipModel();
	~idClipModel();

	void LoadModel(const idTraceModel& trm, bool persistantThroughSave = true);

	void Link(idClip& clp);				// must have been linked with an entity and id before
	void Link(idClip& clp, std::shared_ptr<idEntity> ent, int newId, const Vector2& newOrigin, int renderModelHandle = -1);
	void Unlink(); // unlink from sectors
	/*void					SetPosition(const idVec3& newOrigin, const idMat3& newAxis);	// unlinks the clip model
	void					Translate(const idVec3& translation);							// unlinks the clip model
	//void					Rotate(const idRotation& rotation);							// unlinks the clip model
	void					Enable();						// enable for clipping
	void					Disable();*/					// keep linked but disable for clipping
	void SetEntity(std::shared_ptr<idEntity> newEntity);
	std::shared_ptr<idEntity> GetEntity() const;
	void SetId(int newId);
	int GetId() const;
	//void					SetOwner(idEntity* newOwner);
	std::weak_ptr<idEntity> GetOwner() const;
	const Vector2& GetOrigin() const;
	bool IsTraceModel() const; // returns true if this is a trace model
	int Handle() const; // returns handle used to collide vs this model
private:
	bool enabled; // true if this clip model is used for clipping
	std::shared_ptr<idEntity> entity; // entity using this clip model
	int id; // id for entities that use multiple clip models
	std::weak_ptr<idEntity> owner; // owner of the entity that owns this clip model
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
	int						touchCount;

	void Init(); // initialize
	void Link_r(std::shared_ptr<clipSector_t> node);

	static int AllocTraceModel(const idTraceModel& trm, bool persistantThroughSaves = true);
	static void FreeTraceModel(int traceModelIndex);
	static std::shared_ptr<idTraceModel> GetCachedTraceModel(int traceModelIndex);
	static int GetTraceModelHashKey(const idTraceModel& trm);
};

inline void idClipModel::SetEntity(std::shared_ptr<idEntity> newEntity) {
	entity = newEntity;
}

inline std::shared_ptr<idEntity> idClipModel::GetEntity() const {
	return entity;
}

inline std::weak_ptr<idEntity> idClipModel::GetOwner() const
{
	return owner;
}

inline const Vector2& idClipModel::GetOrigin() const {
	return origin;
}

inline bool idClipModel::IsTraceModel() const
{
	return (traceModelIndex != -1);
}


/*
====================
idClip::ClipModelsTouchingBounds_r
====================
*/
struct listParms_t {
	idBounds		bounds;
	int				contentMask;
	std::shared_ptr<idClipModel>* list;
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

	void Init();
	void Shutdown();

	// clip versus the rest of the world
	bool Translation(trace_t& results, const Vector2& start, const Vector2& end,
		const std::shared_ptr<idClipModel> mdl, int contentMask, const std::shared_ptr<idEntity> passEntity);
	bool Motion(trace_t& results, const Vector2& start, const Vector2& end,
		const std::shared_ptr<idClipModel> mdl, int contentMask, const std::shared_ptr<idEntity> passEntity);

	int Contacts(std::vector<contactInfo_t>& contacts, const int maxContacts, const Vector2& start,
		const Vector2& dir, const float depth, const std::shared_ptr<idClipModel> mdl, int contentMask,
		const std::shared_ptr<idEntity> passEntity);

	int ClipModelsTouchingBounds(const idBounds& bounds, int contentMask, std::shared_ptr<idClipModel>* clipModelList,
		int maxCount) const;

	std::shared_ptr<idClipModel> DefaultClipModel();
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
	void ClipModelsTouchingBounds_r(std::shared_ptr<const clipSector_t> node, listParms_t& parms) const;
	const std::shared_ptr<idTraceModel> TraceModelForClipModel(const std::shared_ptr<idClipModel> mdl) const;
	int GetTraceClipModels(const idBounds& bounds, int contentMask,
		const std::shared_ptr<idEntity> passEntity, std::shared_ptr<idClipModel>* clipModelList) const;
	void TraceRenderModel(trace_t& trace, const Vector2& start, const Vector2& end, const float radius,
		std::shared_ptr<idClipModel> touch) const;
};

inline std::shared_ptr<idClipModel> idClip::DefaultClipModel() {
	return defaultClipModel;
}

#endif // ! CLIP_H
