#include "SimpleObject.h"

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

pos_type::pos_type(std::initializer_list<Screen::pos> il)
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

SimpleObject::SimpleObject(Screen::Pixel drawPix) : drawPixel(drawPix)
{
}

SimpleObject::~SimpleObject()
{
}

Point::Point(Screen::pos pos_h, Screen::pos pos_w, Screen::Pixel drawPix) : SimpleObject(drawPix), pos({ pos_h, pos_w })
{
}

Point::Point(pos_type position, Screen::Pixel drawPix) : SimpleObject(drawPix), pos(position)
{
	int a = 0;
}

Point::~Point()
{
	int a = 0;
}

bool Point::checkCollide(const pos_type pos) const
{
	return getPos() != pos;
}

void Point::update(Scene &scene)
{
}

void Point::drawToScreen(Screen & screen)
{
	screen.set(pos.h, pos.w, drawPixel);
}
