#ifndef POINT_H
#define POINT_H

#include "Vector2.h"
#include "Screen.h"
#include "RenderEntity.h"
#include "AIEntity.h"

/*class SimpleObject
{
public:
	SimpleObject(Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));
	
	virtual ~SimpleObject() = 0;

	virtual void update(Game &Game) = 0;
	virtual void drawToScreen(Screen &screen) = 0;

	virtual void move() = 0;
protected:
	Screen::Pixel drawPixel;
};*/

class Point : public AIEntity
{
public:
	Point(Vector2 position, Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));

	~Point();

	virtual void init() override;
	virtual void update() override;

	/*bool checkCollide(const Game &Game) const override { return true; }
	bool checkCollide(std::shared_ptr<Point> point) override  { return true; }
	bool checkCollide(const pos_type pos) const override;

	void update(Game &Game) override;
	void drawToScreen(Screen &screen) override;

	void move() override {}

	pos_type getPos() const { return pos; }*/
};

#endif