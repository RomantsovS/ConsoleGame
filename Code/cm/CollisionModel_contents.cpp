#include "idlib/precompiled.h"

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
bool idCollisionModelManagerLocal::TestTrmVertsInBrush(cm_traceWork_t* tw,
                                                       cm_brush_t* b) {
  /*if (b->checkcount == idCollisionModelManagerLocal::checkCount) {
          return false;
  }
  b->checkcount = idCollisionModelManagerLocal::checkCount;*/

  if (!(b->contents & tw->contents)) {
    return false;
  }

  // if the brush bounds don't intersect the trace bounds
  if (!b->bounds.IntersectsBounds(tw->bounds) ||
      tw->can_contain && b->bounds.ContainsBounds(tw->bounds)) {
    return false;
  }

  tw->trace.fraction = 0.0f;
  // tw->trace.c.type = CONTACT_TRMVERTEX;
  // tw->trace.c.normal = b->planes[bestPlane].Normal();
  // tw->trace.c.dist = b->planes[bestPlane].Dist();
  tw->trace.c.contents = b->contents;
  // tw->trace.c.material = b->material;
  // tw->trace.c.point = p;
  // tw->trace.c.modelFeature = 0;
  // tw->trace.c.trmFeature = j;

  if (tw->getContacts) {
    CM_AddContact(tw);
  }

  return true;
}

/*
==================
idCollisionModelManagerLocal::ContentsTrm
==================
*/
int idCollisionModelManagerLocal::ContentsTrm(trace_t* results,
                                              const Vector2& start,
                                              const idTraceModel* trm,
                                              int contentMask, int model,
                                              const Vector2& modelOrigin) {
  int i;
  cm_traceWork_t tw;

  // fast point case
  if (!trm || (trm->bounds[1][0] - trm->bounds[0][0] <= 0.0f &&
               trm->bounds[1][1] - trm->bounds[0][1] <= 0.0f &&
               trm->bounds[1][2] - trm->bounds[0][2] <= 0.0f)) {
    // results->c.contents =
    // idCollisionModelManagerLocal::TransformedPointContents(start, model,
    // modelOrigin, modelAxis);
    results->c.contents = contentMask;
    results->fraction = (results->c.contents == 0);
    results->endpos = start;

    return results->c.contents;
  }

  // idCollisionModelManagerLocal::checkCount++;

  tw.trace.fraction = 1.0f;
  tw.trace.c.contents = 0;
  // tw.trace.c.type = CONTACT_NONE;
  tw.contents = contentMask;
  tw.isConvex = true;
  // tw.rotation = false;
  tw.positionTest = true;
  tw.pointTrace = false;
  tw.quickExit = false;
  tw.numContacts = 0;
  tw.model = idCollisionModelManagerLocal::models[model];
  tw.start = start - modelOrigin;
  tw.end = tw.start;
  tw.can_contain = model == 0;

  // setup trm structure
  idCollisionModelManagerLocal::SetupTrm(&tw, trm);

  // calculate vertex positions
  for (i = 0; i < tw.numVerts; i++) {
    // set trm at start position
    tw.vertices[i].p += tw.start;
  }

  // setup trm vertices
  tw.size.Clear();
  for (i = 0; i < tw.numVerts; i++) {
    // get axial trm size after rotations
    tw.size.AddPoint(tw.vertices[i].p - tw.start);
  }

  // bounds for full trace, a little bit larger for epsilons
  for (i = 0; i < 2; i++) {
    if (tw.start[i] < tw.end[i]) {
      tw.bounds[0][i] = tw.start[i] + tw.size[0][i] - CM_BOX_EPSILON;
      tw.bounds[1][i] = tw.end[i] + tw.size[1][i] + CM_BOX_EPSILON;
    } else {
      tw.bounds[0][i] = tw.end[i] + tw.size[0][i] - CM_BOX_EPSILON;
      tw.bounds[1][i] = tw.start[i] + tw.size[1][i] + CM_BOX_EPSILON;
    }
  }

  // trace through the model
  idCollisionModelManagerLocal::TraceThroughModel(&tw);

  *results = tw.trace;
  results->fraction = (results->c.contents == 0);
  results->endpos = start;

  return results->c.contents;
}