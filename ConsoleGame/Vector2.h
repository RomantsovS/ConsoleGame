#ifndef VECTOR2_H
#define VECTOR2_H

#include <initializer_list>
#include <string>

struct Vector2
{
	friend Vector2 operator+(const Vector2 &l, const Vector2&r);
	friend bool operator==(const Vector2 & l, const Vector2 & r);
	friend bool operator!=(const Vector2 & l, const Vector2 & r);

	Vector2() = default;
	Vector2(size_t height, size_t width);
	Vector2(std::initializer_list<size_t> il);

	void Zero();

	Vector2 &operator+=(const Vector2 & r);

	size_t x, y;

	std::string ToString(int precision = 2) const;
};

bool operator==(const Vector2 & l, const Vector2 & r);
bool operator!=(const Vector2 & l, const Vector2 & r);

#endif