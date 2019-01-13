#ifndef SNAKE
#define SNAKE

#include <string>
#include <list>

#include "Screen.h"
#include "SimpleObject.h"
#include "Scene.h"

class Snake : public SimpleObject
{
public:
	Snake(Screen::pos size_h, Screen::pos pos_x = 0, Screen::pos pos_y = 0, char backSy = ' ',
		char drawSy = '*', SimpleObject::directions dir = SimpleObject::DOWN);

	~Snake();

	bool checkCollide(const Scene &scene, SimpleObject::directions param) const;
	bool checkCollide(std::shared_ptr<Point> point, directions dir);
	bool checkCollide(const pos_type pos) const;

	virtual void drawToScreen(Screen &screen);

	void setDirection(SimpleObject::directions dir);

	void move(directions dir);
private:
	char backgroundSymbol;

	std::list<pos_type> points;

	SimpleObject::directions direction;
};

#endif