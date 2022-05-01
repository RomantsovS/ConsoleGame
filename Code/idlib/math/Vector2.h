#ifndef VECTOR2_H
#define VECTOR2_H

struct Vector2 {
	Vector2() = default;
	Vector2(float height, float width) noexcept : x(height), y(width) {}
	Vector2(int height, int width) noexcept : x(static_cast<float>(height)), y(static_cast<float>(width)) {}
	Vector2(std::initializer_list<float> il) {
		if (il.size() != 2)
			throw std::invalid_argument("expected list with 2 members");

		gsl::span<float> ptr{ &x, 2 };
		std::copy(il.begin(), il.end(), ptr.begin());
	}

	void Set(const float x, const float y) noexcept;
	void Zero() noexcept;

	float operator[](const int index) const;
	float& operator[](const int index);
	Vector2 operator*(const float a) const noexcept;
	Vector2 operator+(const Vector2& a) const noexcept;
	Vector2 operator-(const Vector2& a) const noexcept;
	Vector2 operator-() const noexcept;
	Vector2& operator+=(const Vector2& r) noexcept;
	Vector2& operator-=(const Vector2& a) noexcept;
	Vector2& operator*=(const Vector2& r) noexcept;

	Vector2 Cross(const Vector2& a) const noexcept;
	float Length() const noexcept;
	float LengthSqr() const noexcept;
	float Normalize() noexcept; // returns length

	int GetDimension() const noexcept;
	Vector2 GetIntegerVectorFloor() const noexcept;
	Vector2 GetIntegerVectorCeil() const noexcept;
	const float* ToFloatPtr() const noexcept;
	float* ToFloatPtr() noexcept;

	bool Compare(const Vector2& a) const noexcept;							// exact compare, no epsilon

	float x, y;

	std::string ToString(int precision = 2) const;
};

extern Vector2 vec2_origin;

inline Vector2 operator*(const float a, const Vector2 b) noexcept {
	return Vector2(b.x * a, b.y * a);
}

inline bool operator==(const Vector2 & l, const Vector2 &r) noexcept {
	return l.x == r.x && l.y == r.y;
}

inline bool operator!=(const Vector2 & l, const Vector2 &r) noexcept {
	return !(l == r);
}

inline float Vector2::operator[](const int index) const {
	if (index > 1)
		throw std::out_of_range("trying access vector data by index " + std::to_string(index));

	return (&x)[index];
}

inline float& Vector2::operator[](const int index) {
	if (index > 1)
		throw std::out_of_range("trying access vector data by index " + std::to_string(index));

	return (&x)[index];
}

inline Vector2 Vector2::operator*(const float a) const noexcept {
	return Vector2(x * a, y * a);
}

inline Vector2 Vector2::operator+(const Vector2& a) const noexcept {
	return Vector2(x + a.x, y + a.y);
}

inline Vector2 &Vector2::operator*=(const Vector2 & r) noexcept {
	x = x * r.x;
	y = y * r.y;

	return *this;
}

inline Vector2 Vector2::Cross(const Vector2& a) const noexcept {
	return Vector2(y * a.x - x * a.y, x * a.y - y * a.x);
}

inline float Vector2::Length() const noexcept {
	return static_cast<float>(idMath::Sqrt(x * x + y * y));
}

inline float Vector2::LengthSqr() const noexcept {
	return (x * x + y * y);
}

inline float Vector2::Normalize() noexcept {
	float sqrLength, invLength;

	sqrLength = x * x + y * y;// +z * z;
	invLength = idMath::InvSqrt(sqrLength);
	x *= invLength;
	y *= invLength;
	//z *= invLength;
	return invLength * sqrLength;
}

inline int Vector2::GetDimension() const noexcept {
	return 2;
}

inline Vector2 Vector2::GetIntegerVectorFloor() const noexcept {
	Vector2 res(static_cast<float>(floor(x)), static_cast<float>(floor(y)));

	return res;
}

inline Vector2 Vector2::GetIntegerVectorCeil() const noexcept {
	Vector2 res(static_cast<float>(ceil(x)), static_cast<float>(ceil(y)));

	return res;
}

inline const float* Vector2::ToFloatPtr() const noexcept {
	return &x;
}

inline float* Vector2::ToFloatPtr() noexcept {
	return &x;
}

inline Vector2 operator*(const Vector2 & l, const Vector2 & r) noexcept {
	Vector2 res = l;
	res *= r;
	return res;
}

inline Vector2 Vector2::operator-(const Vector2& a) const noexcept {
	return Vector2(x - a.x, y - a.y);
}

inline Vector2 Vector2::operator-() const noexcept {
	return Vector2(-x, -y);
}

inline Vector2 &Vector2::operator+=(const Vector2 & r) noexcept {
	x += r.x;
	y += r.y;

	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& a) noexcept {
	x -= a.x;
	y -= a.y;

	return *this;
}
/*
inline Vector2 operator+(const Vector2 & l, const Vector2 & r)
{
	Vector2 sum = l;
	sum += r;
	return sum;
}
*/
template<typename T>
inline Vector2 operator/(const Vector2& l, const T val) noexcept {
	Vector2 res = l;
	res.x /= val;
	res.y /= val;

	return res;
}

inline void Vector2::Set(const float x, const float y) noexcept {
	this->x = x;
	this->y = y;
}

inline void Vector2::Vector2::Zero() noexcept {
	x = y = 0;
}

inline bool Vector2::Compare(const Vector2& a) const noexcept {
	return x == a.x && y == a.y;
}

inline std::string Vector2::ToString(int precision) const {
	std::string s;

	s = std::to_string(x) + ' ' + std::to_string(y);

	return s;
}

#endif