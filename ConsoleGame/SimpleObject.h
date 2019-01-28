#ifndef SIMPLE_OBJECT_H
#define SIMPLE_OBJECT_H

#include "Screen.h"
#include "RenderEntity.h"

class Game;
class Point;

struct pos_type
{
	friend pos_type operator+(const pos_type &l, const pos_type&r);
	friend bool operator==(const pos_type & l, const pos_type & r);
	friend bool operator!=(const pos_type & l, const pos_type & r);

	pos_type(Screen::pos height, Screen::pos width) : h(height), w(width) {}
	pos_type(std::initializer_list<Screen::pos> il);

	pos_type &operator+=(const pos_type & r);

	Screen::pos h, w;
};

bool operator==(const pos_type & l, const pos_type & r);
bool operator!=(const pos_type & l, const pos_type & r);

class SimpleObject
{
public:
	enum directions {LEFT, RIGHT, UP, DOWN};

	SimpleObject(Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));
	
	virtual ~SimpleObject() = 0;

	virtual bool checkCollide(const Game &Game) const = 0;
	virtual bool checkCollide(std::shared_ptr<Point> point) = 0;
	virtual bool checkCollide(const pos_type pos) const = 0;

	virtual void update(Game &Game) = 0;
	virtual void drawToScreen(Screen &screen) = 0;

	virtual void move() = 0;

	virtual void setActive(bool param)
	{
		active = param;
	}

	virtual bool isActive()
	{
		return active;
	}
protected:
	Screen::Pixel drawPixel;
	bool active;
};

class Point : public SimpleObject
{
public:
	Point(Screen::pos pos_h = 0, Screen::pos pos_w = 0, Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));
	Point(pos_type position, Screen::Pixel drawPix = Screen::Pixel('*', Screen::White));

	~Point();

	bool checkCollide(const Game &Game) const override { return true; }
	bool checkCollide(std::shared_ptr<Point> point) override  { return true; }
	bool checkCollide(const pos_type pos) const override;

	void update(Game &Game) override;
	void drawToScreen(Screen &screen) override;

	void move() override {}

	pos_type getPos() const { return pos; }
private:
	pos_type pos;
};

#endif