#include <stdexcept>

#include "Vector2.h"

bool operator==(const Vector2 &l, const Vector2 &r)
{
	return l.x == r.x && l.y == r.y;
}

bool operator!=(const Vector2 & l, const Vector2 & r)
{
	return !(l == r);
}

Vector2 operator+(const Vector2 & l, const Vector2 & r)
{
	Vector2 sum = l;
	sum += r;
	return sum;
}

Vector2::Vector2(size_t height, size_t width) : x(height), y(width)
{
}

Vector2::Vector2(std::initializer_list<size_t> il)
{
	if (il.size() != 2)
		throw std::invalid_argument("expected list with 2 members");

	auto iter = il.begin();

	x = *iter++;
	y = *iter;
}

void Vector2::Zero()
{
	x = y = 0;
}

Vector2& Vector2::operator+=(const Vector2 & r)
{
	x += r.x;
	y += r.y;

	return *this;
}

std::string Vector2::ToString(int precision) const
{
	std::string s;

	s = std::to_string(x) + ' ' + std::to_string(y);

	return s;
}
