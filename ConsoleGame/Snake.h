#ifndef SNAKE_H
#define SNAKE_H

#include <string>
#include <list>

#include "Screen.h"
#include "SimpleObject.h"
#include "Game.h"

class Snake : public SimpleObject
{
public:
	Snake(Screen::pos size_h, Screen::pos pos_x = 0, Screen::pos pos_y = 0, Screen::Pixel backPix = Screen::Pixel(' ', Screen::Black),
		Screen::Pixel drawPix = Screen::Pixel('*', Screen::White), SimpleObject::directions dir = SimpleObject::DOWN);

	~Snake();

	bool checkCollide(const Game &Game) const override;
	bool checkCollide(std::shared_ptr<Point> point) override;
	bool checkCollide(const pos_type pos) const override;

	void update(Game &Game) override;
	void drawToScreen(Screen &screen) override;

	void move() override;

	void setDirection(SimpleObject::directions dir);
private:
	Screen::Pixel backgroundPixel;

	std::list<pos_type> points;

	SimpleObject::directions direction;
};

#endif