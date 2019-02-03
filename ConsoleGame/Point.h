#ifndef POINT_H
#define POINT_H

#include "Vector2.h"
#include "Screen.h"
#include "AI.h"

/*class SimpleObject
{
public:
	SimpleObject(Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));
	
	virtual ~SimpleObject() = 0;

	virtual void think(Game &Game) = 0;
	virtual void drawToScreen(Screen &screen) = 0;

	virtual void move() = 0;
protected:
	Screen::Pixel drawPixel;
};*/

class Point : public AI
{
public:
	Point(Vector2 position, Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));

	~Point();

	void Spawn();
	virtual void Think() override;

	/*bool checkCollide(const Game &Game) const override { return true; }
	bool checkCollide(std::shared_ptr<Point> point) override  { return true; }
	bool checkCollide(const pos_type pos) const override;

	void think(Game &Game) override;
	void drawToScreen(Screen &screen) override;

	void move() override {}

	pos_type getPos() const { return pos; }*/
};

#endif