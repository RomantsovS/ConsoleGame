#include "CollisionModel_local.h"

/*
===============================================================================

Contents test

===============================================================================
*/

/*
================
idCollisionModelManagerLocal::TestTrmVertsInBrush

  returns true if any of the trm vertices is inside the brush
================
*/
bool idCollisionModelManagerLocal::TestTrmVertsInBrush(cm_traceWork_t* tw, std::shared_ptr<cm_brush_t> b) {
	int j, numVerts;// , bestPlane;
	//float d, bestd;
	Vector2* p;

	/*if (b->checkcount == idCollisionModelManagerLocal::checkCount) {
		return false;
	}
	b->checkcount = idCollisionModelManagerLocal::checkCount;*/

	if (!(b->contents & tw->contents)) {
		return false;
	}

	// if the brush bounds don't intersect the trace bounds
	if (!b->bounds.IntersectsBounds(tw->bounds)) {
		return false;
	}

	if (tw->pointTrace) {
		numVerts = 1;
	}
	else {
		numVerts = tw->numVerts;
	}

	for (j = 0; j < numVerts; j++) {
		p = &tw->vertices[j].p;

		// see if the point is inside the brush
		/*bestPlane = 0;
		bestd = -idMath::INFINITY;
		for (i = 0; i < b->numPlanes; i++) {
			d = b->planes[i].Distance(*p);
			if (d >= 0.0f) {
				break;
			}
			if (d > bestd) {
				bestd = d;
				bestPlane = i;
			}
		}*/
		//if (i >= b->numPlanes) {
		if(b->bounds.LineIntersection(*p, tw->vertices[j].endp))
		{
			tw->trace.fraction = 0.0f;
			tw->trace.c.type = CONTACT_TRMVERTEX;
			//tw->trace.c.normal = b->planes[bestPlane].Normal();
			//tw->trace.c.dist = b->planes[bestPlane].Dist();
			tw->trace.c.contents = b->contents;
			//tw->trace.c.material = b->material;
			tw->trace.c.point = *p;
			tw->trace.c.modelFeature = 0;
			tw->trace.c.trmFeature = j;

			if (tw->getContacts) {
				CM_AddContact(tw);
			}

			return true;
		}
	}
	return false;
}

/*
==================
idCollisionModelManagerLocal::ContentsTrm
==================
*/
int idCollisionModelManagerLocal::ContentsTrm(trace_t* results, const Vector2& start,
	const std::shared_ptr<idTraceModel> trm, int contentMask,
	int model, const Vector2& modelOrigin) {
	results->fraction = (results->c.contents == 0);
	return results->c.contents;
}