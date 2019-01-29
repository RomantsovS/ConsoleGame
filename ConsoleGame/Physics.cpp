#include <stdexcept>

#include "Physics.h"

bool operator==(const pos_type &l, const pos_type &r)
{
	return l.h == r.h && l.w == r.w;
}

bool operator!=(const pos_type & l, const pos_type & r)
{
	return !(l == r);
}

pos_type operator+(const pos_type & l, const pos_type & r)
{
	pos_type sum = l;
	sum += r;
	return sum;
}

pos_type::pos_type(std::initializer_list<size_t> il)
{
	if (il.size() != 2)
		throw std::invalid_argument("expected list with 2 members");

	auto iter = il.begin();

	h = *iter++;
	w = *iter;
}

pos_type& pos_type::operator+=(const pos_type & r)
{
	h += r.h;
	w += r.w;

	return *this;
}

Physics::Physics()
{
}


Physics::~Physics()
{
}
