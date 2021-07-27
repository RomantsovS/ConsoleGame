#include <precompiled.h>
#pragma hdrstop

#include "../Game_local.h"

const size_t MAX_SECTOR_DEPTH = 5;
constexpr size_t MAX_SECTORS = ((1 << (MAX_SECTOR_DEPTH + 1)) - 1);

struct clipSector_t {
	clipSector_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "clipSector_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~clipSector_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "clipSector_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	clipSector_t(const clipSector_t&) = default;
	clipSector_t& operator=(const clipSector_t&) = default;
	clipSector_t(clipSector_t&&) = default;
	clipSector_t& operator=(clipSector_t&&) = default;

	int axis;		// -1 = leaf node
	float dist;
	std::shared_ptr<clipSector_t> children[2];
	clipLink_t* clipLinks = nullptr;
};

struct clipLink_t {
	clipLink_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "clipLink_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~clipLink_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "clipLink_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	clipLink_t(const clipLink_t&) = default;
	clipLink_t& operator=(const clipLink_t&) = default;
	clipLink_t(clipLink_t&&) = default;
	clipLink_t& operator=(clipLink_t&&) = default;

	idClipModel* clipModel;
	clipSector_t* sector;
	clipLink_t* prevInSector;
	clipLink_t* nextInSector;
	std::shared_ptr<clipLink_t> nextLink;
};

struct trmCache_t {
	trmCache_t() : trm(std::make_shared<idTraceModel>()) {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "trmCache_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	~trmCache_t() 	{
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "trmCache_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	trmCache_t(const trmCache_t&) = default;
	trmCache_t& operator=(const trmCache_t&) = default;
	trmCache_t(trmCache_t&&) = default;
	trmCache_t& operator=(trmCache_t&&) = default;

	std::shared_ptr<idTraceModel> trm;
	int						refCount;
	//float					volume;
	//Vector2					centerOfMass;
};

Vector2 vec3_boxEpsilon(CM_BOX_EPSILON, CM_BOX_EPSILON);

static std::vector<std::shared_ptr<trmCache_t>> traceModelCache;
static std::vector<std::shared_ptr<trmCache_t>> traceModelCache_Unsaved;
static std::map<int, int> traceModelHash;
static std::map<int, int> traceModelHash_Unsaved;
const static int TRACE_MODEL_SAVED = BIT(16);

//#define DEBUG_PRINT_Ctor_Dtor

idClipModel::idClipModel() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if (isCommonExists)
		common->DPrintf("%s ctor\n", "idClipModel");
#endif // DEBUG_PRINT_Ctor_Dtor

	Init();
}

/*
================
idClipModel::idClipModel
================
*/
idClipModel::idClipModel(const std::string& name) {
	Init();
	LoadModel(name);
}

idClipModel::idClipModel(const idTraceModel& trm) {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idClipModel");
#endif // DEBUG_PRINT_Ctor_Dtor

	Init();
	LoadModel(trm, true);
}

/*
================
idClipModel::idClipModel
================
*/
idClipModel::idClipModel(const idClipModel& model) {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idClipModel");
#endif // DEBUG_PRINT_Ctor_Dtor

	enabled = model.enabled;
	entity = model.entity;
	id = model.id;
	owner = model.owner;
	origin = model.origin;
	//axis = model.axis;
	bounds = model.bounds;
	absBounds = model.absBounds;
	//material = model.material;
	contents = model.contents;
	collisionModelHandle = model.collisionModelHandle;
	traceModelIndex = -1;
	if (model.traceModelIndex != -1) {
		LoadModel(*GetCachedTraceModel(model.traceModelIndex));
	}
	renderModelHandle = model.renderModelHandle;
	clipLinks = NULL;
	touchCount = -1;
}

idClipModel::~idClipModel() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idClipModel");
#endif // DEBUG_PRINT_Ctor_Dtor

	// make sure the clip model is no longer linked
	Unlink();
	/*if (traceModelIndex != -1) {
		FreeTraceModel(traceModelIndex);
	}*/
}

/*
================
idClipModel::LoadModel
================
*/
bool idClipModel::LoadModel(const std::string& name) {
	renderModelHandle = -1;
	if (traceModelIndex != -1) {
		FreeTraceModel(traceModelIndex);
		traceModelIndex = -1;
	}
	collisionModelHandle = collisionModelManager->LoadModel(name);
	if (collisionModelHandle) {
		collisionModelManager->GetModelBounds(collisionModelHandle, bounds);
		collisionModelManager->GetModelContents(collisionModelHandle, contents);
		return true;
	}
	else {
		bounds.Zero();
		return false;
	}
}

void idClipModel::LoadModel(const idTraceModel& trm, bool persistantThroughSave) {
	collisionModelHandle = 0;
	renderModelHandle = -1;
	if (traceModelIndex != -1) {
		FreeTraceModel(traceModelIndex);
	}
	traceModelIndex = AllocTraceModel(trm, persistantThroughSave);
	bounds = trm.bounds;
}

void idClipModel::Link(idClip& clp) {
	//assert(idClipModel::entity);
	if (idClipModel::entity.expired()) {
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

	Link_r(clp.clipSectors.front().get());
}

void idClipModel::Link(idClip& clp, std::shared_ptr<idEntity> ent, int newId, const Vector2& newOrigin, int renderModelHandle) {
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

/*
============
idClipModel::CheckModel
============
*/
int idClipModel::CheckModel(const std::string& name) {
	return collisionModelManager->LoadModel(name);
}

void idClipModel::Unlink() noexcept  {
	for (auto link = clipLinks; link; link = clipLinks) {
		clipLinks = link->nextLink;
		if (link->prevInSector) {
			link->prevInSector->nextInSector = link->nextInSector;
		}
		else {
			link->sector->clipLinks = link->nextInSector;
		}
		if (link->nextInSector) {
			link->nextInSector->prevInSector = link->prevInSector;
		}
		//link = nullptr;
	}
}

void idClipModel::Init() {
	enabled = true;
	entity.reset();
	id = 0;
	owner.reset();
	origin.Zero();
	//axis.Identity();
	bounds.Zero();
	absBounds.Zero();
	//material = NULL;
	//contents = CONTENTS_BODY;
	collisionModelHandle = 0;
	renderModelHandle = -1;
	traceModelIndex = -1;
	clipLinks = nullptr;
	touchCount = -1;
}

void idClipModel::Link_r(clipSector_t* node) {
	while (node->axis != -1) {
		if (absBounds[0][node->axis] > node->dist) {
			node = node->children[0].get();
		}
		else if (absBounds[1][node->axis] < node->dist) {
			node = node->children[1].get();
		}
		else {
			Link_r(node->children[0].get());
			node = node->children[1].get();
		}
	}

	auto link = std::make_shared<clipLink_t>();

	link->clipModel = this;
	link->sector = node;
	link->nextInSector = node->clipLinks;
	link->prevInSector = nullptr;
	if (node->clipLinks) {
		node->clipLinks->prevInSector = link.get();
	}
	node->clipLinks = link.get();
	link->nextLink = clipLinks;
	clipLinks = link;
}

int idClipModel::AllocTraceModel(const idTraceModel& trm, bool persistantThroughSaves) {
	int traceModelIndex;

	auto hashKey = GetTraceModelHashKey(trm);

	if (persistantThroughSaves) {
		// Look Inside the saved list.
		auto iter = traceModelHash.find(hashKey);
		if (iter != traceModelHash.end() && *traceModelCache[iter->second]->trm == trm)
		{
			traceModelCache[iter->second]->refCount++;
			int flagged_index = iter->second | TRACE_MODEL_SAVED;
			return flagged_index;
		}
	}
	else {

		// Look inside the unsaved list.
		auto iter = traceModelHash_Unsaved.find(hashKey);
		if (iter != traceModelHash_Unsaved.end() && *traceModelCache_Unsaved[iter->second]->trm == trm)
		{
			traceModelCache[iter->second]->refCount++;
		}
	}

	auto entry = std::make_shared<trmCache_t>();
	*entry->trm = trm;
	entry->refCount = 1;

	if (persistantThroughSaves) {
		traceModelCache.push_back(entry);
		traceModelIndex = traceModelCache.size() - 1;
		traceModelHash[hashKey] = traceModelIndex;

		// Set the saved bit.
		traceModelIndex |= TRACE_MODEL_SAVED;
	}
	else {
		traceModelCache.push_back(entry);
		traceModelIndex = traceModelCache.size() - 1;
		traceModelHash[hashKey] = traceModelIndex;

		// Set the saved bit.
		traceModelIndex &= TRACE_MODEL_SAVED;
	}

	return traceModelIndex;
}

void idClipModel::FreeTraceModel(int traceModelIndex) {
	size_t realTraceModelIndex = traceModelIndex & ~TRACE_MODEL_SAVED;

	// Check which cache we are using.
	if (traceModelIndex & TRACE_MODEL_SAVED) {

		if (realTraceModelIndex < 0 || realTraceModelIndex >= traceModelCache.size() || traceModelCache[realTraceModelIndex]->refCount <= 0) {
			gameLocal.Warning("idClipModel::FreeTraceModel: tried to free uncached trace model");
			return;
		}
		traceModelCache[realTraceModelIndex]->refCount--;

	}
	else {

		if (realTraceModelIndex < 0 || realTraceModelIndex >= traceModelCache_Unsaved.size() || traceModelCache_Unsaved[realTraceModelIndex]->refCount <= 0) {
			gameLocal.Warning("idClipModel::FreeTraceModel: tried to free uncached trace model");
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
		gameLocal.Warning("idClipModel::Handle: clip model %d on '%s' (%x) is not a collision or trace model", id, entity.lock()->name.c_str(), entity.lock()->entityNumber);
		return 0;
	}
}

/*
===============
idClipModel::ClearTraceModelCache
===============
*/
void idClipModel::ClearTraceModelCache() noexcept {
	traceModelCache.clear();
	traceModelCache_Unsaved.clear();
	traceModelHash.clear();
	traceModelHash_Unsaved.clear();
}

idTraceModel* idClipModel::GetCachedTraceModel(int traceModelIndex) {
	const int realTraceModelIndex = traceModelIndex & ~TRACE_MODEL_SAVED;

	if (traceModelIndex & TRACE_MODEL_SAVED) {
		return traceModelCache.at(realTraceModelIndex)->trm.get();
	}
	else {
		return traceModelCache_Unsaved.at(realTraceModelIndex)->trm.get();
	}
}

int idClipModel::GetTraceModelHashKey(const idTraceModel& trm) noexcept {
	const auto v = trm.bounds[0];
	return (trm.type << 8) ^ (trm.numVerts << 4) ^ idMath::FloatHash(v.ToFloatPtr(), v.GetDimension());
}

idClip::idClip() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s ctor\n", "idClip");
#endif // DEBUG_PRINT_Ctor_Dtor

	numClipSectors = 0;
	clipSectors.clear();
	worldBounds.Zero();
	numRotations = numTranslations = numMotions = numRenderModelTraces = numContents = numContacts = 0;
}

idClip::~idClip() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idClip");
#endif // DEBUG_PRINT_Ctor_Dtor
}

void idClip::Init() {
	Vector2 size, maxSector = vec2_origin;

	// clear clip sectors
	clipSectors.clear();
	clipSectors.resize(MAX_SECTORS);
	numClipSectors = 0;
	touchCount = -1;
	// get world map bounds
	auto h = collisionModelManager->LoadModel("worldMap");
	collisionModelManager->GetModelBounds(h, worldBounds);
	worldBounds[1] -= vec2_point_size;
	// create world sectors
	CreateClipSectors_r(0, worldBounds, maxSector);

	size = worldBounds[1] - worldBounds[0];
	gameLocal.Printf("map bounds are (%1.1f, %1.1f)\n", size[0], size[1]);
	gameLocal.Printf("max clip sector is (%1.1f, %1.1f)\n", maxSector[0], maxSector[1]);

	// initialize a default clip model
	defaultClipModel = std::make_shared<idClipModel>();
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
	if (defaultClipModel && defaultClipModel->traceModelIndex != -1) {
		idClipModel::FreeTraceModel(defaultClipModel->traceModelIndex);
		defaultClipModel->traceModelIndex = -1;

		defaultClipModel = nullptr;
	}

	//clipLinkAllocator.Shutdown();
}

/*
============
idClip::TestHugeTranslation
============
*/
inline bool TestHugeTranslation(trace_t& results, const idClipModel* mdl, const Vector2& start,
	const Vector2& end) {
	if (mdl && (end - start).LengthSqr() > Square(CM_MAX_TRACE_DIST)) {

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
	const idClipModel* mdl, int contentMask, const idEntity* passEntity) {
	int i, num;
	std::vector<idClipModel*> clipModelList(1);
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
		auto touch = clipModelList[i];

		if (!touch) {
			continue;
		}

		if (touch->renderModelHandle != -1) {
			idClip::numRenderModelTraces++;
			//TraceRenderModel(trace, start, end, radius, touch);
		}
		else {
			idClip::numTranslations++;
			collisionModelManager->Translation(&trace, start, end, trm, contentMask,
				touch->Handle(), touch->origin);
		}

		if (trace.fraction < results.fraction) {
			results = trace;
			results.c.entityNum = touch->entity.lock()->entityNumber;
			results.c.id = touch->id;
			if (results.fraction == 0.0f) {
				break;
			}
		}
	}

	auto res = results.fraction < 1.0f;

	return res;
}

bool idClip::Motion(trace_t& results, const Vector2& start, const Vector2& end,
	const idClipModel* mdl, int contentMask, const idEntity* passEntity)
{
	idClip::numMotions++;

	//int i, num;
	//idClipModel* touch, * clipModelList[MAX_GENTITIES];
	//Vector2 dir, endPosition;
	idBounds traceBounds;
	//float radius;
	//trace_t translationalTrace, rotationalTrace, trace;
	//const idTraceModel* trm;

	if (TestHugeTranslation(results, mdl, start, end)) {
		return true;
	}

	/*if (mdl != NULL && rotation.GetAngle() != 0.0f && rotation.GetVec() != vec3_origin) {
		// if no translation
		if (start == end) {
			// pure rotation
			return Rotation(results, start, rotation, mdl, trmAxis, contentMask, passEntity);
		}
	}
	else*/if (start != end) {
		// pure translation
		return Translation(results, start, end, mdl, contentMask, passEntity);
	}
	else {
		// no motion
		results.fraction = 1.0f;
		results.endpos = start;
		//results.endAxis = trmAxis;
		return false;
	}

	return false;
}

int idClip::Contacts(std::vector<contactInfo_t>& contacts, const int maxContacts, const Vector2& start,
	const Vector2& dir, const float depth, const idClipModel* mdl, int contentMask,
	const idEntity* passEntity) {
	int i, j, num, n, numContacts;
	std::shared_ptr<idClipModel> touch;
	std::vector<idClipModel*> clipModelList(1);
	idBounds traceBounds;

	const idTraceModel* trm = TraceModelForClipModel(mdl);

	if (!passEntity || passEntity->entityNumber != ENTITYNUM_WORLD) {
		// test world
		idClip::numContacts++;
		numContacts = collisionModelManager->Contacts(contacts.data(), maxContacts, start, dir, depth, trm,
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
		auto touch = clipModelList[i];

		if (!touch) {
			continue;
		}

		// no contacts with render models
		if (touch->renderModelHandle != -1) {
			continue;
		}

		idClip::numContacts++;
		n = collisionModelManager->Contacts(contacts.data() + numContacts, maxContacts - numContacts,
			start, dir, depth, trm, contentMask, touch->Handle(), touch->origin);

		for (j = 0; j < n; j++) {
			contacts[numContacts].entityNum = touch->entity.lock()->entityNumber;
			contacts[numContacts].id = touch->id;
			numContacts++;
		}

		if (numContacts >= maxContacts) {
			break;
		}
	}

	return numContacts;
}

int idClip::ClipModelsTouchingBounds(const idBounds& bounds, int contentMask, std::vector<idClipModel*>& clipModelList,
	int maxCount) const
{
	listParms_t parms;

	if (bounds[0][0] > bounds[1][0] ||
		bounds[0][1] > bounds[1][1]
		//||bounds[0][2] > bounds[1][2]
		) {
		// we should not go through the tree for degenerate or backwards bounds
		//assert(false);
		return 0;
	}

	parms.bounds[0] = bounds[0] - vec3_boxEpsilon;
	parms.bounds[1] = bounds[1] + vec3_boxEpsilon;
	parms.contentMask = contentMask;
	parms.list = &clipModelList;
	parms.count = 0;
	parms.maxCount = maxCount;

	touchCount++;
	ClipModelsTouchingBounds_r(clipSectors.front().get(), parms);

	return parms.count;
}

std::shared_ptr<clipSector_t> idClip::CreateClipSectors_r(const int depth, const idBounds& bounds, Vector2& maxSector)
{
	int				i;
	Vector2			size;
	idBounds		front, back;

	clipSectors[idClip::numClipSectors] = std::make_shared<clipSector_t>();

	std::shared_ptr<clipSector_t> anode = clipSectors[idClip::numClipSectors];

	idClip::numClipSectors++;

	if (depth == MAX_SECTOR_DEPTH) {
		anode->axis = -1;
		anode->children[0] = anode->children[1] = nullptr;

		for (i = 0; i < 2; i++) {
			if (bounds[1][i] - bounds[0][i] > maxSector[i]) {
				maxSector[i] = bounds[1][i] - bounds[0][i];
			}
		}
		return anode;
	}

	size = bounds[1] - bounds[0];
	if (size[0] >= size[1]) {
		anode->axis = 0;
	}
	else if (size[1] >= size[0]) {
		anode->axis = 1;
	}
	else {
		common->Error("third dimension on clip sector");
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

void idClip::ClipModelsTouchingBounds_r(const clipSector_t* node, listParms_t& parms) const {
	while (node->axis != -1) {
		if (parms.bounds[0][node->axis] > node->dist) {
			node = node->children[0].get();
		}
		else if (parms.bounds[1][node->axis] < node->dist) {
			node = node->children[1].get();
		}
		else {
			ClipModelsTouchingBounds_r(node->children[0].get(), parms);
			node = node->children[1].get();
		}
	}

	for (clipLink_t* link = node->clipLinks; link; link = link->nextInSector) {
		idClipModel* check = link->clipModel;

		// if the clip model is enabled
		if (!check->enabled) {
			continue;
		}

		// avoid duplicates in the list
		if (check->touchCount == touchCount) {
			continue;
		}

		// if the clip model does not have any contents we are looking for
		/*if (!(check->contents & parms.contentMask)) {
			continue;
		}*/

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

		parms.list->resize(parms.count + 1);
		(*(parms.list))[parms.count] = check;
		parms.count++;
	}
}

const idTraceModel* idClip::TraceModelForClipModel(const idClipModel* mdl) const {
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
	const idEntity* passEntity, std::vector<idClipModel*>& clipModelList) const {
	int i, num;
	idClipModel* cm;
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

	clipModelList.resize(num);

	for (i = 0; i < num; i++) {

		cm = clipModelList[i];

		if (auto entSp = cm->entity.lock()) {
			// check if we should ignore this entity
			if (entSp.get() == passEntity) {
				clipModelList[i] = nullptr;			// don't clip against the pass entity
			}
			else if (entSp == passOwner) {
				clipModelList[i] = NULL;			// missiles don't clip with their owner
			}
			else if (auto ownerSp = cm->owner.lock()) {
				if (ownerSp.get() == passEntity) {
					clipModelList[i] = nullptr;		// don't clip against own missiles
				}
				else if (ownerSp == passOwner) {
					clipModelList[i] = nullptr;		// don't clip against other missiles from same owner
				}
			}
		}
	}

	return num;
}

void idClip::TraceRenderModel(trace_t& trace, const Vector2& start, const Vector2& end, const float radius,
	idClipModel* touch) const {
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

/*
============
idClip::PrintStatistics
============
*/
void idClip::PrintStatistics() noexcept {
	static char buf[256];

	//sprintf_s(buf, "t = %3d, r = %3d, m = %3d, render = %3d, contents = %3d, contacts = %3d",
	//	numTranslations, numRotations, numMotions, numRenderModelTraces, numContents, numContacts);

	//gameRenderWorld->DrawTextToScreen(buf, Vector2(), gameLocal.GetColors()[8], 0);

	numRotations = numTranslations = numMotions = numRenderModelTraces = numContents = numContacts = 0;
}

/*
============
idClip::DrawClipModels
============
*/
void idClip::DrawClipModels(const Vector2& eye, const float radius, const std::shared_ptr<idEntity>& passEntity) {
	int				i, num;
	idBounds		bounds;
	std::vector<idClipModel*> clipModelList(1);

	bounds = idBounds(eye).Expand(radius);

	num = idClip::ClipModelsTouchingBounds(bounds, -1, clipModelList, MAX_GENTITIES);

	for (i = 0; i < num; i++) {
		gsl::not_null<idClipModel*> clipModel = clipModelList[i];
		if (clipModel->GetEntity() == passEntity) {
			continue;
		}
		//if (clipModel->renderModelHandle != -1) {
			gameRenderWorld->DebugBounds(colorCyan, clipModel->GetAbsBounds(), vec2_origin, gameLocal.GetInfoUpdateTime() + 1);
		/* }
		else {
			collisionModelManager->DrawModel(clipModel->Handle(), clipModel->GetOrigin(), clipModel->GetAxis(), eye, radius);
		}*/
	}
}

void idClip::DrawClipSectors() {
	const auto node = clipSectors.front().get();

	DrawClipSectors_r(node, worldBounds);
}

bool idClip::DrawClipSectors_r(const clipSector_t* node, const idBounds& bounds) {
	if (node->axis == -1)
		return false;

	idBounds front, back;

	front = bounds;
	back = bounds;

	front[0][node->axis] = back[1][node->axis] = node->dist;

	if (!DrawClipSectors_r(node->children[0].get(), front)) {
		gameRenderWorld->DebugBounds(colorGreen, front, vec2_origin, gameLocal.GetInfoUpdateTime() + 1);
	}

	if (!DrawClipSectors_r(node->children[1].get(), back)) {
		gameRenderWorld->DebugBounds(colorGreen, back, vec2_origin, gameLocal.GetInfoUpdateTime() + 1);
	}

	return true;
}