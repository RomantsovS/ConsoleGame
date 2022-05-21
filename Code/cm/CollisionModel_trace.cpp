#include "idlib/precompiled.h"

#include "CollisionModel_local.h"

/*
================
idCollisionModelManagerLocal::TraceTrmThroughNode
================
*/
void idCollisionModelManagerLocal::TraceTrmThroughNode(cm_traceWork_t* tw, cm_node_t* node) {
	//cm_polygonRef_t* pref;
	std::shared_ptr<cm_brushRef_t> bref;

	// if already stuck in solid
	if (tw->trace.fraction == 0.0f) {
		return;
	}
	// test if any of the trm vertices is inside a brush
	for (bref = node->brushes; bref; bref = bref->next) {
		if (idCollisionModelManagerLocal::TestTrmVertsInBrush(tw, bref->b.get())) {
			return;
		}
	}
	// if just testing a point we're done
	if (tw->pointTrace) {
		return;
	}
}

void idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(cm_traceWork_t* tw, cm_node_t* node, float p1f, float p2f, Vector2& p1, Vector2& p2) {
	if (!node) {
		return;
	}

	if (tw->quickExit) {
		return;		// stop immediately
	}

	if (tw->trace.fraction <= p1f) {
		return;		// already hit something nearer
	}

	// if we need to test this node for collisions
	if (true || (tw->positionTest && node->brushes)) {
		// trace through node with collision data
		idCollisionModelManagerLocal::TraceTrmThroughNode(tw, node);
	}
	// if already stuck in solid
	if (tw->positionTest && tw->trace.fraction == 0.0f) {
		return;
	}
	// if this is a leaf node
	if (node->planeType == -1) {
		return;
	}

	idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, node->children[0].get(), p1f, p2f, p1, p2);
	idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, node->children[1].get(), p1f, p2f, p1, p2);
	
	return;
}

/*
================
idCollisionModelManagerLocal::TraceThroughModel
================
*/
void idCollisionModelManagerLocal::TraceThroughModel(cm_traceWork_t* tw) {
	// trace through spatial subdivision and then through leafs
	idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, tw->model.lock()->node.get(), 0, 1, tw->start, tw->end);
}