#ifndef PHYSICS_H
#define PHYSICS_H

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

