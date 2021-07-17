#ifndef  BOUNDS_H
#define BOUNDS_H

class idBounds {
public:
	idBounds();
	explicit idBounds(const Vector2& mins, const Vector2& maxs);
	explicit idBounds(const Vector2& point);

	const Vector2& operator[](const int index) const;
	Vector2& operator[](const int index);
	idBounds operator+(const Vector2& t) const;				// returns translated bounds
	idBounds& operator+=(const Vector2& t);					// translate the bounds
	idBounds& operator+=(const idBounds& a);

	bool Compare(const idBounds& a) const;							// exact compare, no epsilon
	bool operator==(const idBounds& a) const;						// exact compare, no epsilon
	bool operator!=(const idBounds& a) const;						// exact compare, no epsilon

	void Clear();									// inside out bounds
	void Zero(); // single point at origin

	float GetRadius() const; // returns the radius relative to the bounds origin
	bool IsCleared() const; // returns true if bounds are inside out

	bool AddPoint(const Vector2& v); // add the point, returns true if the bounds expanded
	bool AddBounds(const idBounds& a); // add the bounds, returns true if the bounds expanded
	idBounds Expand(const float d) const; // return bounds expanded in all directions with the given value
	idBounds& ExpandSelf(const float d); // expand bounds in all directions with the given value

	bool ContainsPoint(const Vector2& p) const;			// includes touching
	bool IntersectsBounds(const idBounds& a) const;	// includes touching
	bool ContainsBounds(const idBounds& a) const;	// includes touching
	bool LineIntersection(const Vector2& start, const Vector2& end) const;

					// most tight bounds for the given transformed bounds
	void FromTransformedBounds(const idBounds& bounds, const Vector2& origin);
	// most tight bounds for a translation
	void FromPointTranslation(const Vector2& point, const Vector2& translation);
	void FromBoundsTranslation(const idBounds& bounds, const Vector2& origin, const Vector2& translation);

	static idBounds GetBoundsZero() {
		return idBounds(vec2_origin, vec2_origin);
	}
private:
	Vector2 b[2];
};

inline idBounds::idBounds() {
}

inline idBounds::idBounds(const Vector2& mins, const Vector2& maxs) {
	b[0] = mins;
	b[1] = maxs;
}

inline idBounds::idBounds(const Vector2& point) {
	b[0] = point;
	b[1] = point;
}

inline const Vector2& idBounds::operator[](const int index) const {
	return b[index];
}

inline Vector2& idBounds::operator[](const int index) {
	return b[index];
}

inline idBounds idBounds::operator+(const Vector2& t) const {
	return idBounds(b[0] + t, b[1] + t);
}

inline idBounds& idBounds::operator+=(const Vector2& t) {
	b[0] += t;
	b[1] += t;
	return *this;
}

inline idBounds& idBounds::operator+=(const idBounds& a) {
	idBounds::AddBounds(a);
	return *this;
}

inline bool idBounds::Compare(const idBounds& a) const {
	return (b[0].Compare(a.b[0]) && b[1].Compare(a.b[1]));
}

inline bool idBounds::operator==(const idBounds& a) const
{
	return Compare(a);
}

inline bool idBounds::operator!=(const idBounds& a) const {
	return !Compare(a);
}

inline void idBounds::Clear() {
	b[0][0] = b[0][1] = idMath::INFINITY;
	b[1][0] = b[1][1] = -idMath::INFINITY;
}

inline void idBounds::Zero() {
	b[0][0] = b[0][1] = b[1][0] = b[1][1] = 0;
}

inline bool idBounds::IsCleared() const {
	return b[0][0] > b[1][0];
}

inline bool idBounds::AddPoint(const Vector2& v) {
	bool expanded = false;
	if (v[0] < b[0][0]) {
		b[0][0] = v[0];
		expanded = true;
	}
	if (v[0] > b[1][0]) {
		b[1][0] = v[0];
		expanded = true;
	}
	if (v[1] < b[0][1]) {
		b[0][1] = v[1];
		expanded = true;
	}
	if (v[1] > b[1][1]) {
		b[1][1] = v[1];
		expanded = true;
	}
	return expanded;
}

inline bool idBounds::AddBounds(const idBounds& a) {
	bool expanded = false;
	if (a.b[0][0] < b[0][0]) {
		b[0][0] = a.b[0][0];
		expanded = true;
	}
	if (a.b[0][1] < b[0][1]) {
		b[0][1] = a.b[0][1];
		expanded = true;
	}
	/*if (a.b[0][2] < b[0][2]) {
		b[0][2] = a.b[0][2];
		expanded = true;
	}*/
	if (a.b[1][0] > b[1][0]) {
		b[1][0] = a.b[1][0];
		expanded = true;
	}
	if (a.b[1][1] > b[1][1]) {
		b[1][1] = a.b[1][1];
		expanded = true;
	}
	/*if (a.b[1][2] > b[1][2]) {
		b[1][2] = a.b[1][2];
		expanded = true;
	}*/
	return expanded;
}

inline idBounds idBounds::Expand(const float d) const {
	return idBounds(Vector2(b[0][0] - d, b[0][1] - d),
		Vector2(b[1][0] + d, b[1][1] + d));
}

inline idBounds& idBounds::ExpandSelf(const float d) {
	b[0][0] -= d;
	b[0][1] -= d;
	b[1][0] += d;
	b[1][1] += d;
	return *this;
}

inline bool idBounds::ContainsPoint(const Vector2& p) const {
	if (p[0] < b[0][0] || p[1] < b[0][1] //|| p[2] < b[0][2]
		|| p[0] > b[1][0] || p[1] > b[1][1] /*|| p[2] > b[1][2]*/) {
		return false;
	}
	return true;
}

inline bool idBounds::IntersectsBounds(const idBounds& a) const {
	if (a.b[1][0] < b[0][0] || a.b[1][1] < b[0][1] /*|| a.b[1][2] < b[0][2]*/
		|| a.b[0][0] > b[1][0] || a.b[0][1] > b[1][1] /*|| a.b[0][2] > b[1][2]*/) {
		return false;
	}
	return true;
}

inline bool idBounds::ContainsBounds(const idBounds& a) const {
	return b[0][0] < a.b[0][0] && b[0][1] < a.b[0][1] &&
		b[1][0] > a.b[1][0] && b[1][1] > a.b[1][1];
}

//extern idBounds	bounds_zero;

#endif // ! BOUNDS_H
