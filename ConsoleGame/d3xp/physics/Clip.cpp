#include "Clip.h"
#include "../../idlib/sys/sys_types.h"
#include "../Game_local.h"
#include "../../cm/CollisionModel.h"
#include "../../idlib/math/Math.h"
#include "../../idlib/math/Vector2.h"

#define	MAX_SECTOR_DEPTH				12
#define MAX_SECTORS						((1<<(MAX_SECTOR_DEPTH+1))-1)

Vector2 vec3_boxEpsilon(CM_BOX_EPSILON, CM_BOX_EPSILON);

static std::vector<std::shared_ptr<trmCache_t>> traceModelCache;
static std::vector<std::shared_ptr<trmCache_t>> traceModelCache_Unsaved;
static std::map<int, int> traceModelHash;
static std::map<int, int> traceModelHash_Unsaved;
const static int TRACE_MODEL_SAVED = BIT(16);

idClipModel::idClipModel()
{
	Init();
}

idClipModel::~idClipModel()
{
	// make sure the clip model is no longer linked
	Unlink();
	/*if (traceModelIndex != -1) {
		FreeTraceModel(traceModelIndex);
	}*/
}

void idClipModel::LoadModel(const idTraceModel& trm, bool persistantThroughSave)
{
	collisionModelHandle = 0;
	renderModelHandle = -1;
	if (traceModelIndex != -1) {
		FreeTraceModel(traceModelIndex);
	}
	traceModelIndex = AllocTraceModel(trm, persistantThroughSave);
	bounds = trm.bounds;
}

void idClipModel::Link(idClip& clp)
{
	//assert(idClipModel::entity);
	if (!idClipModel::entity) {
		return;
	}

	if (clipLinks) {
		Unlink();	// unlink from old position
	}

	if (bounds.IsCleared()) {
		return;
	}

	// set the abs box
	/*if (axis.IsRotated()) {
		// expand for rotation
		absBounds.FromTransformedBounds(bounds, origin, axis);
	}
	else {*/
		// normal
		absBounds[0] = bounds[0] + origin;
		absBounds[1] = bounds[1] + origin;
	//}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	absBounds[0] -= vec3_boxEpsilon;
	absBounds[1] += vec3_boxEpsilon;

	Link_r(clp.clipSectors.front());
}

void idClipModel::Link(idClip& clp, std::shared_ptr<idEntity> ent, int newId, const Vector2& newOrigin, int renderModelHandle)
{
	this->entity = ent;
	this->id = newId;
	this->origin = newOrigin;
	//this->axis = newAxis;
	if (renderModelHandle != -1) {
		this->renderModelHandle = renderModelHandle;
		const renderEntity_t* renderEntity = gameRenderWorld->GetRenderEntity(renderModelHandle);
		if (renderEntity) {
			this->bounds = renderEntity->bounds;
		}
	}
	this->Link(clp);
}

void idClipModel::Unlink()
{
	for (auto link = clipLinks; link; link = clipLinks) {
		clipLinks = link->nextLink;
		if (!link->prevInSector.expired()) {
			link->prevInSector.lock()->nextInSector = link->nextInSector;
		}
		else {
			link->sector->clipLinks = link->nextInSector;
		}
		if (link->nextInSector) {
			link->nextInSector->prevInSector = link->prevInSector;
		}
		//clipLinkAllocator.Free(link);
	}
}

inline void idClipModel::SetId(int newId)
{
	id = newId;
}

inline int idClipModel::GetId() const
{
	return id;
}

void idClipModel::Init()
{
	enabled = true;
	entity = nullptr;
	id = 0;
	owner.reset();
	origin.Zero();
	//axis.Identity();
	//bounds.Zero();
	//absBounds.Zero();
	//material = NULL;
	//contents = CONTENTS_BODY;
	//collisionModelHandle = 0;
	renderModelHandle = -1;
	traceModelIndex = -1;
	clipLinks = NULL;
	touchCount = -1;
}

void idClipModel::Link_r(std::shared_ptr<clipSector_t> node)
{
	while (node->axis != -1) {
		if (absBounds[0][node->axis] > node->dist) {
			node = node->children[0];
		}
		else if (absBounds[1][node->axis] < node->dist) {
			node = node->children[1];
		}
		else {
			Link_r(node->children[0]);
			node = node->children[1];
		}
	}

	auto link = std::make_shared<clipLink_t>();
	link->clipModel = shared_from_this();
	link->sector = node;
	link->nextInSector = node->clipLinks.lock();
	link->prevInSector.reset();
	if (!node->clipLinks.expired()) {
		node->clipLinks.lock()->prevInSector = link;
	}
	node->clipLinks = link;
	link->nextLink = clipLinks;
	clipLinks = link;
}

int idClipModel::AllocTraceModel(const idTraceModel& trm, bool persistantThroughSaves)
{
	int i, traceModelIndex;

	auto hashKey = GetTraceModelHashKey(trm);

	if (persistantThroughSaves) {
		// Look Inside the saved list.
		i = traceModelHash.at(hashKey);
		if (i && *traceModelCache[i]->trm == trm)
		{
			traceModelCache[i]->refCount++;
		}
	}
	else {

		// Look inside the unsaved list.
		i = traceModelHash_Unsaved.at(hashKey);
		if (i && *traceModelCache_Unsaved[i]->trm == trm)
		{
			traceModelCache_Unsaved[i]->refCount++;
		}
	}

	auto entry = std::make_shared<trmCache_t>();
	*entry->trm = trm;
	entry->refCount = 1;

	if (persistantThroughSaves) {
		traceModelCache.push_back(entry);
		traceModelIndex = traceModelCache.size() - 1;
		traceModelHash[hashKey, traceModelIndex];

		// Set the saved bit.
		traceModelIndex |= TRACE_MODEL_SAVED;
	}
	else {
		traceModelCache.push_back(entry);
		traceModelIndex = traceModelCache.size() - 1;
		traceModelHash[hashKey, traceModelIndex];

		// Set the saved bit.
		traceModelIndex &= TRACE_MODEL_SAVED;
	}

	return traceModelIndex;
}

void idClipModel::FreeTraceModel(int traceModelIndex)
{
	size_t realTraceModelIndex = traceModelIndex & ~TRACE_MODEL_SAVED;

	// Check which cache we are using.
	if (traceModelIndex & TRACE_MODEL_SAVED) {

		if (realTraceModelIndex < 0 || realTraceModelIndex >= traceModelCache.size() || traceModelCache[realTraceModelIndex]->refCount <= 0) {
			std::logic_error("idClipModel::FreeTraceModel: tried to free uncached trace model");
			return;
		}
		traceModelCache[realTraceModelIndex]->refCount--;

	}
	else {

		if (realTraceModelIndex < 0 || realTraceModelIndex >= traceModelCache_Unsaved.size() || traceModelCache_Unsaved[realTraceModelIndex]->refCount <= 0) {
			std::logic_error("idClipModel::FreeTraceModel: tried to free uncached trace model");
			return;
		}
		traceModelCache_Unsaved[realTraceModelIndex]->refCount--;

	}
}

/*
================
idClipModel::Handle
================
*/
int idClipModel::Handle() const {
	//assert(renderModelHandle == -1);
	if (collisionModelHandle) {
		return collisionModelHandle;
	}
	else if (traceModelIndex != -1) {
		return collisionModelManager->SetupTrmModel(*GetCachedTraceModel(traceModelIndex)/*, material*/);
	}
	else {
		// this happens in multiplayer on the combat models
		gameLocal.Warning("idClipModel::Handle: clip model %d on '%s' (%x) is not a collision or trace model", id, entity->name.c_str(), entity->entityNumber);
		return 0;
	}
}

std::shared_ptr<idTraceModel> idClipModel::GetCachedTraceModel(int traceModelIndex)
{
	int realTraceModelIndex = traceModelIndex & ~TRACE_MODEL_SAVED;

	if (traceModelIndex & TRACE_MODEL_SAVED) {
		return traceModelCache[realTraceModelIndex]->trm;
	}
	else {
		return traceModelCache_Unsaved[realTraceModelIndex]->trm;
	}
}

int idClipModel::GetTraceModelHashKey(const idTraceModel& trm)
{
	const auto v = trm.bounds[0];
	return (trm.type << 8) ^ (trm.numVerts << 4) ^ idMath::FloatHash(v.ToFloatPtr(), v.GetDimension());
}

idClip::idClip()
{
	numClipSectors = 0;
	clipSectors.clear();
	worldBounds.Zero();
	numRotations = numTranslations = numMotions = numRenderModelTraces = numContents = numContacts = 0;
	defaultClipModel = std::make_shared<idClipModel>();
}

void idClip::Init()
{
	Vector2 size, maxSector = vec2_origin;

	// clear clip sectors
	clipSectors.clear();
	clipSectors.resize(MAX_SECTORS);
	numClipSectors = 0;
	touchCount = -1;
	// get world map bounds
	auto h = collisionModelManager->LoadModel("worldMap");
	collisionModelManager->GetModelBounds(h, worldBounds);
	// create world sectors
	CreateClipSectors_r(0, worldBounds, maxSector);

	size = worldBounds[1] - worldBounds[0];
	//gameLocal.Printf("map bounds are (%1.1f, %1.1f, %1.1f)\n", size[0], size[1], size[2]);
	//gameLocal.Printf("max clip sector is (%1.1f, %1.1f, %1.1f)\n", maxSector[0], maxSector[1], maxSector[2]);

	// initialize a default clip model
	defaultClipModel->LoadModel(idTraceModel(idBounds(Vector2(0, 0)).Expand(8)));

	// set counters to zero
	numRotations = numTranslations = numMotions = numRenderModelTraces = numContents = numContacts = 0;
}

void idClip::Shutdown()
{
	clipSectors.clear();

	// free the trace model used for the temporaryClipModel
	if (temporaryClipModel.traceModelIndex != -1) {
		idClipModel::FreeTraceModel(temporaryClipModel.traceModelIndex);
		temporaryClipModel.traceModelIndex = -1;
	}

	// free the trace model used for the defaultClipModel
	if (defaultClipModel->traceModelIndex != -1) {
		idClipModel::FreeTraceModel(defaultClipModel->traceModelIndex);
		defaultClipModel->traceModelIndex = -1;
	}

	//clipLinkAllocator.Shutdown();
}

/*
============
idClip::TestHugeTranslation
============
*/
inline bool TestHugeTranslation(trace_t& results, const std::shared_ptr<idClipModel> mdl, const Vector2& start,
	const Vector2& end) {
	if (mdl != NULL && (end - start).LengthSqr() > Square(CM_MAX_TRACE_DIST)) {

		results.fraction = 0.0f;
		results.endpos = start;
		//results.endAxis = trmAxis;
		memset(&results.c, 0, sizeof(results.c));
		results.c.point = start;

		if (mdl->GetEntity()) {
			gameLocal.Printf("huge translation for clip model %d on entity %d '%s'\n", mdl->GetId(), mdl->GetEntity()->entityNumber, mdl->GetEntity()->GetName());
		}
		else {
			gameLocal.Printf("huge translation for clip model %d\n", mdl->GetId());
		}
		return true;
	}
	return false;
}

bool idClip::Translation(trace_t& results, const Vector2& start, const Vector2& end,
	const std::shared_ptr<idClipModel> mdl, int contentMask, const std::shared_ptr<idEntity> passEntity)
{
	int i, num;
	std::shared_ptr<idClipModel> touch, clipModelList[MAX_GENTITIES];
	idBounds traceBounds;
	float radius;
	trace_t trace;

	if (TestHugeTranslation(results, mdl, start, end)) {
		return true;
	}

	const auto trm = TraceModelForClipModel(mdl);

	if (!passEntity || passEntity->entityNumber != ENTITYNUM_WORLD) {
		// test world
		idClip::numTranslations++;
		collisionModelManager->Translation(&results, start, end, trm, contentMask, 0, vec2_origin);
		results.c.entityNum = results.fraction != 1.0f ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
		if (results.fraction == 0.0f) {
			return true;		// blocked immediately by the world
		}
	}
	else {
		memset(&results, 0, sizeof(results));
		results.fraction = 1.0f;
		results.endpos = end;
		//results.endAxis = trmAxis;
	}

	if (!trm) {
		traceBounds.FromPointTranslation(start, results.endpos - start);
		radius = 0;
	}
	else {
		traceBounds.FromBoundsTranslation(trm->bounds, start, results.endpos - start);
		radius = trm->bounds.GetRadius();
	}

	num = GetTraceClipModels(traceBounds, contentMask, passEntity, clipModelList);

	for (i = 0; i < num; i++) {
		touch = clipModelList[i];

		if (!touch) {
			continue;
		}

		if (touch->renderModelHandle != -1) {
			idClip::numRenderModelTraces++;
			TraceRenderModel(trace, start, end, radius, touch);
		}
		else {
			idClip::numTranslations++;
			collisionModelManager->Translation(&trace, start, end, trm, contentMask,
				touch->Handle(), touch->origin);
		}

		if (trace.fraction < results.fraction) {
			results = trace;
			results.c.entityNum = touch->entity->entityNumber;
			results.c.id = touch->id;
			if (results.fraction == 0.0f) {
				break;
			}
		}
	}

	return (results.fraction < 1.0f);
}

bool idClip::Motion(trace_t& results, const Vector2& start, const Vector2& end,
	const std::shared_ptr<idClipModel> mdl, int contentMask, const std::shared_ptr<idEntity> passEntity)
{
	/*int i, num;
	idClipModel* touch, * clipModelList[MAX_GENTITIES];
	Vector2 dir, endPosition;
	idBounds traceBounds;
	float radius;
	trace_t translationalTrace, rotationalTrace, trace;
	const idTraceModel* trm;

	if (TestHugeTranslation(results, mdl, start, end)) {
		return true;
	}*/

	/*if (mdl != NULL && rotation.GetAngle() != 0.0f && rotation.GetVec() != vec3_origin) {
		// if no translation
		if (start == end) {
			// pure rotation
			return Rotation(results, start, rotation, mdl, trmAxis, contentMask, passEntity);
		}
	}
	else*//*if (start != end) {
		// pure translation
		return Translation(results, start, end, mdl, contentMask, passEntity);
	}
	else {
		// no motion
		results.fraction = 1.0f;
		results.endpos = start;
		//results.endAxis = trmAxis;
		return false;
	}*/

	return false;
}

int idClip::Contacts(std::vector<contactInfo_t>& contacts, const int maxContacts, const Vector2& start,
	const Vector2& dir, const float depth, const std::shared_ptr<idClipModel> mdl, int contentMask,
	const std::shared_ptr<idEntity> passEntity)
{
	int i, j, num, n, numContacts;
	std::shared_ptr<idClipModel> touch, clipModelList[MAX_GENTITIES];
	idBounds traceBounds;

	const std::shared_ptr<idTraceModel> trm = TraceModelForClipModel(mdl);

	if (!passEntity || passEntity->entityNumber != ENTITYNUM_WORLD) {
		// test world
		idClip::numContacts++;
		numContacts = collisionModelManager->Contacts(contacts.begin(), maxContacts, start, dir, depth, trm,
			contentMask, 0, vec2_origin);
	}
	else {
		numContacts = 0;
	}

	for (i = 0; i < numContacts; i++) {
		contacts[i].entityNum = ENTITYNUM_WORLD;
		contacts[i].id = 0;
	}

	if (numContacts >= maxContacts) {
		return numContacts;
	}

	if (!trm) {
		traceBounds = idBounds(start).Expand(depth);
	}
	else {
		traceBounds.FromTransformedBounds(trm->bounds, start);
		traceBounds.ExpandSelf(depth);
	}

	num = GetTraceClipModels(traceBounds, contentMask, passEntity, clipModelList);

	for (i = 0; i < num; i++) {
		touch = clipModelList[i];

		if (!touch) {
			continue;
		}

		// no contacts with render models
		if (touch->renderModelHandle != -1) {
			continue;
		}

		idClip::numContacts++;
		n = collisionModelManager->Contacts(contacts.begin() + numContacts, maxContacts - numContacts,
			start, dir, depth, trm, contentMask, touch->Handle(), touch->origin);

		for (j = 0; j < n; j++) {
			contacts[numContacts].entityNum = touch->entity->entityNumber;
			contacts[numContacts].id = touch->id;
			numContacts++;
		}

		if (numContacts >= maxContacts) {
			break;
		}
	}

	return numContacts;
}

int idClip::ClipModelsTouchingBounds(const idBounds& bounds, int contentMask, std::shared_ptr<idClipModel>* clipModelList,
	int maxCount) const
{
	listParms_t parms;

	if (bounds[0][0] > bounds[1][0] ||
		bounds[0][1] > bounds[1][1]
		//||bounds[0][2] > bounds[1][2]
		) {
		// we should not go through the tree for degenerate or backwards bounds
		throw std::logic_error("");
		return 0;
	}

	parms.bounds[0] = bounds[0] - vec3_boxEpsilon;
	parms.bounds[1] = bounds[1] + vec3_boxEpsilon;
	parms.contentMask = contentMask;
	parms.list = clipModelList;
	parms.count = 0;
	parms.maxCount = maxCount;

	touchCount++;
	ClipModelsTouchingBounds_r(clipSectors.front(), parms);

	return parms.count;
}

std::shared_ptr<clipSector_t> idClip::CreateClipSectors_r(const int depth, const idBounds& bounds,
 Vector2& maxSector)
{
	int				i;
	std::shared_ptr<clipSector_t> anode;
	Vector2			size;
	idBounds		front, back;

	anode = clipSectors[idClip::numClipSectors];
	idClip::numClipSectors++;

	if (depth == MAX_SECTOR_DEPTH) {
		anode->axis = -1;
		anode->children[0] = anode->children[1] = nullptr;

		for (i = 0; i < 3; i++) {
			if (bounds[1][i] - bounds[0][i] > maxSector[i]) {
				maxSector[i] = bounds[1][i] - bounds[0][i];
			}
		}
		return anode;
	}

	size = bounds[1] - bounds[0];
	if (size[0] >= size[1] && size[0] >= size[2]) {
		anode->axis = 0;
	}
	else if (size[1] >= size[0] && size[1] >= size[2]) {
		anode->axis = 1;
	}
	else {
		anode->axis = 2;
	}

	anode->dist = 0.5f * (bounds[1][anode->axis] + bounds[0][anode->axis]);

	front = bounds;
	back = bounds;

	front[0][anode->axis] = back[1][anode->axis] = anode->dist;

	anode->children[0] = CreateClipSectors_r(depth + 1, front, maxSector);
	anode->children[1] = CreateClipSectors_r(depth + 1, back, maxSector);

	return anode;
}

void idClip::ClipModelsTouchingBounds_r(std::shared_ptr<const clipSector_t> node, listParms_t& parms) const
{
	while (node->axis != -1) {
		if (parms.bounds[0][node->axis] > node->dist) {
			node = node->children[0];
		}
		else if (parms.bounds[1][node->axis] < node->dist) {
			node = node->children[1];
		}
		else {
			ClipModelsTouchingBounds_r(node->children[0], parms);
			node = node->children[1];
		}
	}

	for (std::shared_ptr<clipLink_t> link = node->clipLinks.lock(); link; link = link->nextInSector) {
		std::shared_ptr<idClipModel> check = link->clipModel.lock();

		// if the clip model is enabled
		if (!check->enabled) {
			continue;
		}

		// avoid duplicates in the list
		if (check->touchCount == touchCount) {
			continue;
		}

		// if the clip model does not have any contents we are looking for
		if (!(check->contents & parms.contentMask)) {
			continue;
		}

		// if the bounds really do overlap
		if (check->absBounds[0][0] > parms.bounds[1][0] ||
			check->absBounds[1][0] < parms.bounds[0][0] ||
			check->absBounds[0][1] > parms.bounds[1][1] ||
			check->absBounds[1][1] < parms.bounds[0][1]// ||
			//check->absBounds[0][2] > parms.bounds[1][2] ||
			//check->absBounds[1][2] < parms.bounds[0][2]
			)
		{
			continue;
		}

		if (parms.count >= parms.maxCount) {
			gameLocal.Warning("idClip::ClipModelsTouchingBounds_r: max count");
			return;
		}

		check->touchCount = touchCount;
		parms.list[parms.count] = check;
		parms.count++;
	}
}

const std::shared_ptr<idTraceModel> idClip::TraceModelForClipModel(const std::shared_ptr<idClipModel> mdl) const
{
	if (!mdl) {
		return nullptr;
	}
	else {
		if (!mdl->IsTraceModel()) {
			if (mdl->GetEntity()) {
				gameLocal.Warning("TraceModelForClipModel: clip model %d on '%s' is not a trace model\n" , mdl->GetId(), mdl->GetEntity()->name.c_str());
			}
			else {
				gameLocal.Warning("TraceModelForClipModel: clip model %d is not a trace model\n" , mdl->GetId());
			}
		}
		return idClipModel::GetCachedTraceModel(mdl->traceModelIndex);
	}
}

int idClip::GetTraceClipModels(const idBounds& bounds, int contentMask,
	const std::shared_ptr<idEntity> passEntity, std::shared_ptr<idClipModel>* clipModelList) const
{
	int i, num;
	std::shared_ptr<idClipModel> cm;
	std::shared_ptr<idEntity> passOwner;

	num = ClipModelsTouchingBounds(bounds, contentMask, clipModelList, MAX_GENTITIES);

	if (!passEntity) {
		return num;
	}

	if (passEntity->GetPhysics()->GetNumClipModels() > 0) {
		passOwner = passEntity->GetPhysics()->GetClipModel()->GetOwner().lock();
	}
	else {
		passOwner = nullptr;
	}

	for (i = 0; i < num; i++) {

		cm = clipModelList[i];

		// check if we should ignore this entity
		if (cm->entity == passEntity) {
			clipModelList[i] = nullptr;			// don't clip against the pass entity
		}
		else if (cm->entity == passOwner) {
			clipModelList[i] = NULL;			// missiles don't clip with their owner
		}
		else if (!cm->owner.expired()) {
			if (cm->owner.lock() == passEntity) {
				clipModelList[i] = nullptr;		// don't clip against own missiles
			}
			else if (cm->owner.lock() == passOwner) {
				clipModelList[i] = nullptr;		// don't clip against other missiles from same owner
			}
		}
	}

	return num;
}

void idClip::TraceRenderModel(trace_t& trace, const Vector2& start, const Vector2& end, const float radius,
	std::shared_ptr<idClipModel> touch) const
{
	trace.fraction = 1.0f;

	// if the trace is passing through the bounds
	if (touch->absBounds.Expand(radius).LineIntersection(start, end)) {
		modelTrace_t modelTrace;

		// test with exact render model and modify trace_t structure accordingly
		if (gameRenderWorld->ModelTrace(modelTrace, touch->renderModelHandle, start, end, radius)) {
			trace.fraction = modelTrace.fraction;
			//trace.endAxis = axis;
			trace.endpos = modelTrace.point;
			//trace.c.normal = modelTrace.normal;
			//trace.c.dist = modelTrace.point * modelTrace.normal;
			trace.c.point = modelTrace.point;
			trace.c.type = CONTACT_TRMVERTEX;
			trace.c.modelFeature = 0;
			trace.c.trmFeature = 0;
			//trace.c.contents = modelTrace.material->GetContentFlags();
			//trace.c.material = modelTrace.material;
			// NOTE: trace.c.id will be the joint number
			touch->id = JOINT_HANDLE_TO_CLIPMODEL_ID(modelTrace.jointNumber);
		}
	}
}
