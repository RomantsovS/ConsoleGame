#ifndef PHYSICS_H
#define PHYSICS_H

#include <initializer_list>

struct pos_type
{
	friend pos_type operator+(const pos_type &l, const pos_type&r);
	friend bool operator==(const pos_type & l, const pos_type & r);
	friend bool operator!=(const pos_type & l, const pos_type & r);

	pos_type(size_t height, size_t width) : h(height), w(width) {}
	pos_type(std::initializer_list<size_t> il);

	pos_type &operator+=(const pos_type & r);

	size_t h, w;
};

bool operator==(const pos_type & l, const pos_type & r);
bool operator!=(const pos_type & l, const pos_type & r);

class Physics
{
public:
	enum directions { LEFT, RIGHT, UP, DOWN };

	Physics();

	virtual ~Physics() = 0;

	/*virtual bool checkCollide(const Game &Game) const = 0;
	virtual bool checkCollide(std::shared_ptr<Point> point) = 0;
	virtual bool checkCollide(const pos_type pos) const = 0;*/
};

#endif

