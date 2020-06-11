#include "Bounds.h"
#include "../math/Math.h"

idBounds bounds_zero(vec2_origin, vec2_origin);
//idBounds bounds_zeroOneCube(idVec3(0.0f), idVec3(1.0f));
//idBounds bounds_unitCube(idVec3(-1.0f), idVec3(1.0f));

/*
============
idBounds::GetRadius
============
*/
float idBounds::GetRadius() const {
	int i;
	float total, b0, b1;

	total = 0;
	for (i = 0; i < 2; i++) {
		b0 = idMath::Fabs(b[0][i]);
		b1 = idMath::Fabs(b[1][i]);
		if (b0 > b1) {
			total += b0 * b0;
		}
		else {
			total += b1 * b1;
		}
	}
	return idMath::Sqrt(total);
}

/*
============
idBounds::LineIntersection

  Returns true if the line intersects the bounds between the start and end point.
============
*/
bool idBounds::LineIntersection(const Vector2& start, const Vector2& end) const {
	/*const Vector2 center = (b[0] + b[1]) * 0.5f;
	const Vector2 extents = b[1] - center;
	const Vector2 lineDir = 0.5f * (end - start);
	const Vector2 lineCenter = start + lineDir;
	const Vector2 dir = lineCenter - center;

	const float ld0 = idMath::Fabs(lineDir[0]);
	if (idMath::Fabs(dir[0]) > extents[0] + ld0) {
		return false;
	}

	const float ld1 = idMath::Fabs(lineDir[1]);
	if (idMath::Fabs(dir[1]) > extents[1] + ld1) {
		return false;
	}

	const float ld2 = idMath::Fabs(lineDir[2]);
	if (idMath::Fabs(dir[2]) > extents[2] + ld2) {
		return false;
	}

	const Vector2 cross = lineDir.Cross(dir);

	if (idMath::Fabs(cross[0]) > extents[1] * ld2 + extents[2] * ld1) {
		return false;
	}

	if (idMath::Fabs(cross[1]) > extents[0] * ld2 + extents[2] * ld0) {
		return false;
	}

	if (idMath::Fabs(cross[2]) > extents[0] * ld1 + extents[1] * ld0) {
		return false;
	}*/

	bool contain_start = ContainsPoint(start);
	bool contain_end = ContainsPoint(end);

	return (contain_start && !contain_end) ||
		(!contain_start && contain_end);

	return true;
}

/*
============
idBounds::FromTransformedBounds
============
*/
void idBounds::FromTransformedBounds(const idBounds& bounds, const Vector2& origin) {
	//int i;
	Vector2 center, extents;// , rotatedExtents;

	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;

	/*for (i = 0; i < 3; i++) {
		rotatedExtents[i] = idMath::Fabs(extents[0] * axis[0][i]) +
			idMath::Fabs(extents[1] * axis[1][i]) +
			idMath::Fabs(extents[2] * axis[2][i]);
	}*/

	center = origin + center;// * axis;
	b[0] = center;// -rotatedExtents;
	b[1] = center;// +rotatedExtents;
}

void idBounds::FromPointTranslation(const Vector2& point, const Vector2& translation)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (translation[i] < 0.0f) {
			b[0][i] = point[i] + translation[i];
			b[1][i] = point[i];
		}
		else {
			b[0][i] = point[i];
			b[1][i] = point[i] + translation[i];
		}
	}
}

void idBounds::FromBoundsTranslation(const idBounds& bounds, const Vector2& origin, const Vector2& translation)
{
	int i;

	/*if (axis.IsRotated()) {
		FromTransformedBounds(bounds, origin, axis);
	}
	else*/ {
		b[0] = bounds[0] + origin;
		b[1] = bounds[1] + origin;
	}
	for (i = 0; i < 2; i++) {
		if (translation[i] < 0.0f) {
			b[0][i] += translation[i];
		}
		else {
			b[1][i] += translation[i];
		}
	}
}
