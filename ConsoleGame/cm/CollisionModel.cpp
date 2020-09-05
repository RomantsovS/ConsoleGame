#include "CollisionModel_local.h"

const float CM_CLIP_EPSILON = 0.05f;			// always stay this distance away from any model
const float CM_BOX_EPSILON = 0.2f;			// should always be larger than clip epsilon
const float CM_POINT_SIZE = 1.0f;
//const float CM_POINT_BOX = CM_POINT_SIZE - 0.1f;
const float CM_MAX_TRACE_DIST = 4096.0f;	// maximum distance a trace model may be traced, point traces are unlimited

const int MAX_SUBMODELS = 2048;
const size_t TRACE_MODEL_HANDLE = MAX_SUBMODELS;

const size_t NODE_BLOCK_SIZE_SMALL = 1;// 8;

Vector2 vec3_pointEpsilon(CM_BOX_EPSILON, CM_BOX_EPSILON);
Vector2 vec2_point_size(CM_POINT_SIZE, CM_POINT_SIZE);