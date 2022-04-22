#include <precompiled.h>
#pragma hdrstop

#include "CollisionModel_local.h"

/*
================
CM_AddContact
================
*/
void CM_AddContact(gsl::not_null<cm_traceWork_t*> tw) {

	if (tw->numContacts >= tw->maxContacts) {
		return;
	}
	// copy contact information from trace_t
	tw->contacts[tw->numContacts] = tw->trace.c;
	tw->numContacts++;
	// set fraction back to 1 to find all other contacts
	tw->trace.fraction = 1.0f;
}

/*
================
idCollisionModelManagerLocal::SetupTrm
================
*/
void idCollisionModelManagerLocal::SetupTrm(cm_traceWork_t* tw, const idTraceModel* trm) noexcept {
	int i;

	// vertices
	tw->numVerts = trm->numVerts;
	for (i = 0; i < trm->numVerts; i++) {
		tw->vertices[i].p = trm->verts[i];
		tw->vertices[i].used = false;
	}
	// is the trace model convex or not
	tw->isConvex = trm->isConvex;
}

void idCollisionModelManagerLocal::Translation(trace_t* results, const Vector2& start, const Vector2& end,
	const idTraceModel* trm, int contentMask, int model, const Vector2& modelOrigin) {
	Vector2 last_step_dist, cur_step_dist;
	int cur_step;

	auto dir = end - start;

	int max_dimension = idMath::Fabs(dir.x) < idMath::Fabs(dir.y);
	
	int num_steps = static_cast<int>(std::ceil(idMath::Fabs(dir[max_dimension]) / CM_POINT_SIZE));
	
	if (num_steps > 1) {
		Vector2 step_dist = dir / num_steps;
		if (step_dist.LengthSqr() >= vec3_pointEpsilon.LengthSqr()) {
			num_steps = static_cast<int>(num_steps / CM_BOX_EPSILON);
			step_dist = dir / num_steps;
		}

		for (last_step_dist = vec2_origin, cur_step_dist = step_dist, cur_step = 0; cur_step < num_steps;
			last_step_dist = cur_step_dist, cur_step_dist += step_dist) {
			TranslationIter(results, start + last_step_dist, start + cur_step_dist, trm, contentMask, model, modelOrigin);
			// if there is a collision
			if (results->fraction < 1.0f) {
				// fraction of total translation
				results->fraction = (last_step_dist[max_dimension] + step_dist[max_dimension] * results->fraction) / dir[max_dimension];
				return;
			}
			++cur_step;
		}
		results->fraction = 1.0f;
		return;
	}

	TranslationIter(results, start, start + dir, trm, contentMask, model, modelOrigin);
}

void idCollisionModelManagerLocal::TranslationIter(trace_t* results, const Vector2& start, const Vector2& end,
	const idTraceModel* trm, int contentMask, int model, const Vector2& modelOrigin) {
	int i;
	cm_trmVertex_t* vert;
	static cm_traceWork_t tw;

	memset(results, 0, sizeof(*results));

	if (model < 0 || model > MAX_SUBMODELS || model > idCollisionModelManagerLocal::maxModels) {
		common->Printf("idCollisionModelManagerLocal::Translation: invalid model handle\n");
		return;
	}
	if (!idCollisionModelManagerLocal::models[model]) {
		common->Printf("idCollisionModelManagerLocal::Translation: invalid model\n");
		return;
	}

	// if case special position test
	if (start[0] == end[0] && start[1] == end[1]) {
		idCollisionModelManagerLocal::ContentsTrm(results, start, trm, contentMask, model, modelOrigin);
		return;
	}

	//idCollisionModelManagerLocal::checkCount++;

	tw.trace.fraction = 1.0f;
	tw.trace.c.contents = 0;
	tw.trace.c.type = CONTACT_NONE;
	tw.contents = contentMask;
	tw.isConvex = true;
	//tw.rotation = false;
	tw.positionTest = false;
	tw.quickExit = false;
	tw.getContacts = idCollisionModelManagerLocal::getContacts;
	tw.contacts = &idCollisionModelManagerLocal::contacts[0];
	tw.maxContacts = idCollisionModelManagerLocal::maxContacts;
	tw.numContacts = 0;
	tw.model = idCollisionModelManagerLocal::models[model];
	tw.start = start - modelOrigin;// .GetIntegerVectorFloor();
	tw.end = end - modelOrigin;// .GetIntegerVectorFloor();
	tw.dir = end - start;

	// the trace fraction is too inaccurate to describe translations over huge distances
	if (tw.dir.LengthSqr() > Square(CM_MAX_TRACE_DIST)) {
		results->fraction = 0.0f;
		results->endpos = start;
		//results->endAxis = trmAxis;
		//results->c.normal = vec3_origin;
		//results->c.material = NULL;
		results->c.point = start;
		/*if (common->RW()) {
			common->RW()->DebugArrow(colorRed, start, end, 1);
		}*/
		common->Printf("idCollisionModelManagerLocal::Translation: huge translation\n");
		return;
	}

	tw.pointTrace = false;
	tw.size.Clear();

	// setup trm structure
	idCollisionModelManagerLocal::SetupTrm(&tw, trm);

	for (i = 0; i < tw.numVerts; i++) {
		// set trm at start position
		tw.vertices[i].p += tw.start;
		tw.vertices[i].used = true;
	}

	// setup trm vertices
	for (vert = tw.vertices, i = 0; i < tw.numVerts; i++, vert++) {
		if (!vert->used) {
			continue;
		}
		// get axial trm size after rotations
		tw.size.AddPoint(vert->p - tw.start);
		// calculate the end position of each vertex for a full trace
		vert->endp = vert->p + tw.dir;

		for (size_t j = 0; j < 2; j++) {
			if (tw.start[j] < tw.end[j]) {
				vert->endp[j] += CM_CLIP_EPSILON;
			}
			else if (tw.start[j] > tw.end[j]) {
				vert->endp[j] -= CM_CLIP_EPSILON;
			}

			if (vert->p[j] == CM_POINT_SIZE) {
				vert->p[j] += CM_CLIP_EPSILON;
			}
		}

		// pluecker coordinate for vertex movement line
		//vert->pl.FromRay(vert->p, tw.dir);
	}

	// bounds for full trace, a little bit larger for epsilons
	for (i = 0; i < 2; i++) {
		if (tw.start[i] < tw.end[i]) {
			tw.bounds[0][i] = tw.start[i] + tw.size[0][i] - CM_CLIP_EPSILON;
			tw.bounds[1][i] = tw.end[i] + tw.size[1][i] + CM_CLIP_EPSILON;
		}
		else /*if (tw.start[i] > tw.end[i])*/ {
			tw.bounds[0][i] = tw.end[i] + tw.size[0][i] - CM_CLIP_EPSILON;
			tw.bounds[1][i] = tw.start[i] + tw.size[1][i] + CM_CLIP_EPSILON;
		}
		/*else {
			tw.bounds[0][i] = tw.start[i];
			tw.bounds[1][i] = tw.start[i];

			if (tw.start[i] >= CM_POINT_SIZE) {
				tw.bounds[0][i] += CM_CLIP_EPSILON;
				tw.bounds[1][i] += CM_CLIP_EPSILON;
			}
		}*/
		/*if (idMath::Fabs(tw.size[0][i]) > idMath::Fabs(tw.size[1][i])) {
			tw.extents[i] = idMath::Fabs(tw.size[0][i]) + CM_BOX_EPSILON;
		}
		else {
			tw.extents[i] = idMath::Fabs(tw.size[1][i]) + CM_BOX_EPSILON;
		}*/
	}

	tw.positionTest = true;

	// trace through the model
	idCollisionModelManagerLocal::TraceThroughModel(&tw);

	// if we're getting contacts
	if (tw.getContacts) {
		// move all contacts to world space
		/*if (model_rotated) {
			for (i = 0; i < tw.numContacts; i++) {
				tw.contacts[i].normal *= modelAxis;
				tw.contacts[i].point *= modelAxis;
			}
		}*/
		if (modelOrigin != vec2_origin) {
			for (i = 0; i < tw.numContacts; i++) {
				tw.contacts[i].point += modelOrigin;
				//tw.contacts[i].dist += modelOrigin * tw.contacts[i].normal;
			}
		}
		idCollisionModelManagerLocal::numContacts = tw.numContacts;
	}
	else {
		// store results
		*results = tw.trace;
		results->endpos = start + results->fraction * (end - start);
		//results->endAxis = trmAxis;

		if (results->fraction < 1.0f) {
			// if the fraction is tiny the actual movement could end up zero
			if (results->fraction > 0.0f && results->endpos.Compare(start)) {
				results->fraction = 0.0f;
			}
			// rotate trace plane normal if there was a collision with a rotated model
			/*if (model_rotated) {
				results->c.normal *= modelAxis;
				results->c.point *= modelAxis;
			}*/
			results->c.point += modelOrigin;
			//results->c.dist += modelOrigin * results->c.normal;
		}
	}
}