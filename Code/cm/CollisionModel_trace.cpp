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

	// position test
	//if (tw->positionTest) {
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
		// test if the trm is stuck in any polygons
		/*for (pref = node->polygons; pref; pref = pref->next) {
			if (idCollisionModelManagerLocal::TestTrmInPolygon(tw, pref->p)) {
				return;
			}
		}*/
	/*}
	else if (tw->rotation) {
		// rotate through all polygons in this leaf
		for (pref = node->polygons; pref; pref = pref->next) {
			if (idCollisionModelManagerLocal::RotateTrmThroughPolygon(tw, pref->p)) {
				return;
			}
		}
	}
	else {
		// trace through all polygons in this leaf
		for (auto pref = node->polygons; pref; pref = pref->next) {
			if (idCollisionModelManagerLocal::TranslateTrmThroughPolygon(tw, pref->p)) {
				return;
			}
		}
	}*/
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
	if (/*node->polygons ||*/true || (tw->positionTest&& node->brushes)) {
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
	//float d;
	//int i, numSteps;
	//Vector2 start, end;
	//idRotation rot;

	//if (!tw->rotation) {
		// trace through spatial subdivision and then through leafs
		idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, tw->model.lock()->node.get(), 0, 1, tw->start, tw->end);
	/*}
	else {
		// approximate the rotation with a series of straight line movements
		// total length covered along circle
		d = tw->radius * DEG2RAD(tw->angle);
		// if more than one step
		if (d > CIRCLE_APPROXIMATION_LENGTH) {
			// number of steps for the approximation
			numSteps = (int)(CIRCLE_APPROXIMATION_LENGTH / d);
			// start of approximation
			start = tw->start;
			// trace circle approximation steps through the BSP tree
			for (i = 0; i < numSteps; i++) {
				// calculate next point on approximated circle
				rot.Set(tw->origin, tw->axis, tw->angle * ((float)(i + 1) / numSteps));
				end = start * rot;
				// trace through spatial subdivision and then through leafs
				idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, tw->model->node, 0, 1, start, end);
				// no need to continue if something was hit already
				if (tw->trace.fraction < 1.0f) {
					return;
				}
				start = end;
			}
		}
		else {
			start = tw->start;
		}
		// last step of the approximation
		idCollisionModelManagerLocal::TraceThroughAxialBSPTree_r(tw, tw->model->node, 0, 1, start, tw->end);
	}*/
}