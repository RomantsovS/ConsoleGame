#ifndef TRACEMODEL_H
#define TRACEMODEL_H

#include "../bv/Bounds.h"

// trace model type
enum traceModel_t {
	TRM_INVALID,		// invalid trm
	TRM_BOX,			// box
	TRM_OCTAHEDRON,		// octahedron
	TRM_DODECAHEDRON,	// dodecahedron
	TRM_CYLINDER,		// cylinder approximation
	TRM_CONE,			// cone approximation
	TRM_BONE,			// two tetrahedrons attached to each other
	TRM_POLYGON,		// arbitrary convex polygon
	TRM_POLYGONVOLUME,	// volume for arbitrary convex polygon
	TRM_CUSTOM			// loaded from map model or ASE/LWO
};

// these are bit cache limits
#define MAX_TRACEMODEL_VERTS		32
#define MAX_TRACEMODEL_EDGES		32

using traceModelVert_t = Vector2;

class idTraceModel {
public:
	traceModel_t type;
	int numVerts;
	traceModelVert_t verts[MAX_TRACEMODEL_VERTS];
	//int numEdges;
	//traceModelEdge_t edges[MAX_TRACEMODEL_EDGES + 1];

	idBounds bounds;
	bool isConvex; // true when model is convex
public:
	idTraceModel();
	// axial bounding box
	idTraceModel(const idBounds& boxBounds);

	// axial box
	void SetupBox(const idBounds& boxBounds);

	// compare
	bool Compare(const idTraceModel& trm) const;
	bool operator==(const idTraceModel& trm) const;
	bool operator!=(const idTraceModel& trm) const;
private:
	void InitBox();
};

inline idTraceModel::idTraceModel()
{
	type = TRM_INVALID;
	numVerts = 0;
	bounds.Zero();
}

inline idTraceModel::idTraceModel(const idBounds& boxBounds)
{
	InitBox();
	SetupBox(boxBounds);
}

inline void idTraceModel::SetupBox(const idBounds& boxBounds)
{
	int i;

	if (type != TRM_BOX) {
		InitBox();
	}
	// offset to center
	//offset = (boxBounds[0] + boxBounds[1]) * 0.5f;
	// set box vertices
	for (i = 0; i < 4; i++) {
		verts[i][0] = boxBounds[(i ^ (i >> 1)) & 1][0];
		verts[i][1] = boxBounds[(i >> 1) & 1][1];
		//verts[i][2] = boxBounds[(i >> 2) & 1][2];
	}
	// set polygon plane distances
	/*polys[0].dist = -boxBounds[0][2];
	polys[1].dist = boxBounds[1][2];
	polys[2].dist = -boxBounds[0][1];
	polys[3].dist = boxBounds[1][0];
	polys[4].dist = boxBounds[1][1];
	polys[5].dist = -boxBounds[0][0];
	// set polygon bounds
	for (i = 0; i < 6; i++) {
		polys[i].bounds = boxBounds;
	}
	polys[0].bounds[1][2] = boxBounds[0][2];
	polys[1].bounds[0][2] = boxBounds[1][2];
	polys[2].bounds[1][1] = boxBounds[0][1];
	polys[3].bounds[0][0] = boxBounds[1][0];
	polys[4].bounds[0][1] = boxBounds[1][1];
	polys[5].bounds[1][0] = boxBounds[0][0];*/

}

inline bool idTraceModel::Compare(const idTraceModel& trm) const
{
	int i;

	if (type != trm.type || numVerts != trm.numVerts) {
		return false;
	}
	if (bounds != trm.bounds) {
		return false;
	}

	switch (type) {
	case TRM_INVALID:
	case TRM_BOX:
	case TRM_OCTAHEDRON:
	case TRM_DODECAHEDRON:
	case TRM_CYLINDER:
	case TRM_CONE:
		break;
	case TRM_BONE:
	case TRM_POLYGON:
	case TRM_POLYGONVOLUME:
	case TRM_CUSTOM:
		for (i = 0; i < trm.numVerts; i++) {
			if (verts[i] != trm.verts[i]) {
				return false;
			}
		}
		break;
	}
	return true;
}

inline bool idTraceModel::operator==(const idTraceModel& trm) const {
	return Compare(trm);
}

inline bool idTraceModel::operator!=(const idTraceModel& trm) const {
	return !Compare(trm);
}

inline void idTraceModel::InitBox()
{
	//int i;

	type = TRM_BOX;
	numVerts = 4;

	// set box edges
	/*for (i = 0; i < 4; i++) {
		edges[i + 1].v[0] = i;
		edges[i + 1].v[1] = (i + 1) & 3;
		edges[i + 5].v[0] = 4 + i;
		edges[i + 5].v[1] = 4 + ((i + 1) & 3);
		edges[i + 9].v[0] = i;
		edges[i + 9].v[1] = 4 + i;
	}

	// all edges of a polygon go counter clockwise
	polys[0].numEdges = 4;
	polys[0].edges[0] = -4;
	polys[0].edges[1] = -3;
	polys[0].edges[2] = -2;
	polys[0].edges[3] = -1;
	polys[0].normal.Set(0.0f, 0.0f, -1.0f);

	polys[1].numEdges = 4;
	polys[1].edges[0] = 5;
	polys[1].edges[1] = 6;
	polys[1].edges[2] = 7;
	polys[1].edges[3] = 8;
	polys[1].normal.Set(0.0f, 0.0f, 1.0f);

	polys[2].numEdges = 4;
	polys[2].edges[0] = 1;
	polys[2].edges[1] = 10;
	polys[2].edges[2] = -5;
	polys[2].edges[3] = -9;
	polys[2].normal.Set(0.0f, -1.0f, 0.0f);

	polys[3].numEdges = 4;
	polys[3].edges[0] = 2;
	polys[3].edges[1] = 11;
	polys[3].edges[2] = -6;
	polys[3].edges[3] = -10;
	polys[3].normal.Set(1.0f, 0.0f, 0.0f);

	polys[4].numEdges = 4;
	polys[4].edges[0] = 3;
	polys[4].edges[1] = 12;
	polys[4].edges[2] = -7;
	polys[4].edges[3] = -11;
	polys[4].normal.Set(0.0f, 1.0f, 0.0f);

	polys[5].numEdges = 4;
	polys[5].edges[0] = 4;
	polys[5].edges[1] = 9;
	polys[5].edges[2] = -8;
	polys[5].edges[3] = -12;
	polys[5].normal.Set(-1.0f, 0.0f, 0.0f);*/

	// convex model
	isConvex = true;

	//GenerateEdgeNormals();
}


#endif // ! TRACEMODEL_H
