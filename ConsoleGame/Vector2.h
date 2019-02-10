#ifndef VECTOR2_H
#define VECTOR2_H

#include <initializer_list>
#include <string>

struct Vector2
{
	Vector2() = default;
	Vector2(size_t height, size_t width) : x(height), y(width) {}
	Vector2(std::initializer_list<size_t> il)
	{
		if (il.size() != 2)
			throw std::invalid_argument("expected list with 2 members");

		auto iter = il.begin();

		x = *iter++;
		y = *iter;
	}

	Vector2 &operator*=(const Vector2 & r);
	Vector2 &operator+=(const Vector2 & r);

	void Zero();

	size_t x, y;

	std::string ToString(int precision = 2) const;
};

inline bool operator==(const Vector2 & l, const Vector2 &r)
{
	return l.x == r.x && l.y == r.y;
}

inline bool operator!=(const Vector2 & l, const Vector2 &r)
{
	return !(l == r);
}

inline Vector2 &Vector2::operator*=(const Vector2 & r)
{
	x = x * r.x;
	y = y * r.y;

	return *this;
}

inline Vector2 operator*(const Vector2 & l, const Vector2 & r)
{
	Vector2 res = l;
	res *= r;
	return res;
}

inline Vector2 &Vector2::operator+=(const Vector2 & r)
{
	x += r.x;
	y += r.y;

	return *this;
}

inline Vector2 operator+(const Vector2 & l, const Vector2 & r)
{
	Vector2 sum = l;
	sum += r;
	return sum;
}

inline void Vector2::Vector2::Zero()
{
	x = y = 0;
}

inline std::string Vector2::ToString(int precision) const
{
	std::string s;

	s = std::to_string(x) + ' ' + std::to_string(y);

	return s;
}

#endif