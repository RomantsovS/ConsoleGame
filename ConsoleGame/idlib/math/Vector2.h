#ifndef VECTOR2_H
#define VECTOR2_H

struct Vector2
{
	Vector2() = default;
	Vector2(float height, float width) : x(height), y(width) {}
	Vector2(int height, int width) : x(static_cast<float>(height)), y(static_cast<float>(width)) {}
	Vector2(std::initializer_list<float> il)
	{
		if (il.size() != 2)
			throw std::invalid_argument("expected list with 2 members");

		auto iter = il.begin();

		x = *iter++;
		y = *iter;
	}

	void Set(const float x, const float y);
	void Zero();

	float operator[](const int index) const;
	float& operator[](const int index);
	Vector2 operator*(const float a) const;
	Vector2 operator-(const Vector2& a) const;
	Vector2 operator-() const;
	Vector2 &operator+=(const Vector2 & r);
	Vector2& operator-=(const Vector2& a);
	Vector2& operator*=(const Vector2& r);

	Vector2 Cross(const Vector2& a) const;
	float LengthSqr() const;
	float Normalize(); // returns length

	int GetDimension() const;
	Vector2 GetIntegerVectorFloor() const;
	Vector2 GetIntegerVectorCeil() const;
	const float* ToFloatPtr() const;
	float* ToFloatPtr();

	bool Compare(const Vector2& a) const;							// exact compare, no epsilon

	float x, y;

	std::string ToString(int precision = 2) const;
};

extern Vector2 vec2_origin;

inline Vector2 operator*(const float a, const Vector2 b) {
	return Vector2(b.x * a, b.y * a);
}

inline bool operator==(const Vector2 & l, const Vector2 &r)
{
	return l.x == r.x && l.y == r.y;
}

inline bool operator!=(const Vector2 & l, const Vector2 &r)
{
	return !(l == r);
}

inline float Vector2::operator[](const int index) const
{
	if (index > 1)
		throw std::out_of_range("trying access vector data by index " + std::to_string(index));

	return (&x)[index];
}

inline float& Vector2::operator[](const int index)
{
	if (index > 1)
		throw std::out_of_range("trying access vector data by index " + std::to_string(index));

	return (&x)[index];
}

inline Vector2 Vector2::operator*(const float a) const
{
	return Vector2(x * a, y * a);
}

inline Vector2 &Vector2::operator*=(const Vector2 & r)
{
	x = x * r.x;
	y = y * r.y;

	return *this;
}

inline Vector2 Vector2::Cross(const Vector2& a) const
{
	return Vector2(y * a.x - x * a.y, x * a.y - y * a.x);
}

inline float Vector2::LengthSqr() const
{
	return (x * x + y * y);
}

inline float Vector2::Normalize() {
	float sqrLength, invLength;

	sqrLength = x * x + y * y;// +z * z;
	invLength = idMath::InvSqrt(sqrLength);
	x *= invLength;
	y *= invLength;
	//z *= invLength;
	return invLength * sqrLength;
}

inline int Vector2::GetDimension() const
{
	return 2;
}

inline Vector2 Vector2::GetIntegerVectorFloor() const
{
	Vector2 res(static_cast<float>(floor(x)), static_cast<float>(floor(y)));

	return res;
}

inline Vector2 Vector2::GetIntegerVectorCeil() const
{
	Vector2 res(static_cast<float>(ceil(x)), static_cast<float>(ceil(y)));

	return res;
}

inline const float* Vector2::ToFloatPtr() const
{
	return &x;
}

inline float* Vector2::ToFloatPtr()
{
	return &x;
}

inline Vector2 operator*(const Vector2 & l, const Vector2 & r)
{
	Vector2 res = l;
	res *= r;
	return res;
}

inline Vector2 Vector2::operator-(const Vector2& a) const
{
	return Vector2(x - a.x, y - a.y);
}

inline Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}

inline Vector2 &Vector2::operator+=(const Vector2 & r)
{
	x += r.x;
	y += r.y;

	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& a)
{
	x -= a.x;
	y -= a.y;

	return *this;
}

inline Vector2 operator+(const Vector2 & l, const Vector2 & r)
{
	Vector2 sum = l;
	sum += r;
	return sum;
}

template<typename T>
inline Vector2 operator/(const Vector2& l, const T val)
{
	Vector2 res = l;
	res.x /= val;
	res.y /= val;

	return res;
}

inline void Vector2::Set(const float x, const float y)
{
	this->x = x;
	this->y = y;
}

inline void Vector2::Vector2::Zero()
{
	x = y = 0;
}

inline bool Vector2::Compare(const Vector2& a) const
{
	return x == a.x && y == a.y;
}

inline std::string Vector2::ToString(int precision) const
{
	std::string s;

	s = std::to_string(x) + ' ' + std::to_string(y);

	return s;
}

#endif