#include "Snake.h"

Snake::Snake(Screen::pos size_h, Screen::pos pos_x, Screen::pos pos_y, Screen::Pixel backPix, Screen::Pixel drawPix, directions dir) :
	SimpleObject(drawPix), backgroundPixel(backPix), direction(dir)
{
	for (size_t i = 0; i != size_h; ++i)
	{
		points.push_back({ pos_x + i, pos_y });
	}
}

Snake::~Snake()
{
}

void Snake::drawToScreen(Screen & screen)
{
	size_t cnt = 0;

	for (auto iter = points.cbegin(); iter != points.cend(); ++iter)
	{
		screen.set(iter->h, iter->w, Screen::Pixel(static_cast<char>(48 + cnt++ % 10), drawPixel.color));
	}
}

void Snake::setDirection(SimpleObject::directions dir)
{
	if (dir == UP)
	{
		if (direction == DOWN)
		{
			points.reverse();
		}

		direction = dir;
	}
	else if (dir == DOWN)
	{
		if (direction == UP)
		{
			points.reverse();
		}

		direction = dir;
	}
	else if (dir == LEFT)
	{
		if (direction == RIGHT)
		{
			points.reverse();
		}

		direction = dir;
	}
	else if (dir == RIGHT)
	{
		if (direction == LEFT)
		{
			points.reverse();
		}

		direction = dir;
	}
}

void Snake::move()
{
	pos_type pos(0, 0);

	if (direction == UP)
		pos = { points.back().h - 1, points.back().w + 0 };
	else if (direction == DOWN)
		pos = { points.back().h + 1, points.back().w + 0 };
	else if (direction == LEFT)
		pos = { points.back().h + 0, points.back().w - 1 };
	else if (direction == RIGHT)
		pos = { points.back().h + 0, points.back().w + 1 };

	points.push_back(pos);
	
	points.pop_front();
}

bool Snake::checkCollide(const Scene &scene) const
{
	auto backPoint = points.back();

	if (direction == SimpleObject::UP)
	{
		pos_type newPos(backPoint.h - 1, backPoint.w);

		return backPoint.h > scene.getBorderHeight() && std::find(points.cbegin(), points.cend(), newPos) == points.cend();
	}
	else if (direction == SimpleObject::DOWN)
	{
		pos_type newPos = {backPoint.h + 1, backPoint.w };

		return backPoint.h < scene.getUsedHeight() && std::find(points.cbegin(), points.cend(), newPos) == points.cend();
	}
	else if (direction == SimpleObject::LEFT)
	{
		pos_type newPos = {backPoint.h, backPoint.w - 1};

		auto p = std::find(points.cbegin(), points.cend(), newPos) == points.cend();

		return backPoint.w > scene.getBorderWidth() && p;
	}
	else if (direction == SimpleObject::RIGHT)
	{
		pos_type newPos = {backPoint.h, backPoint.w + 1};

		return backPoint.w < scene.getUsedWidth() && std::find(points.cbegin(), points.cend(), newPos) == points.cend();
	}

	return false;
}

bool Snake::checkCollide(std::shared_ptr<Point> point)
{
	auto pos = *(--points.end());

	if (direction == UP)
		pos += pos_type(-1, 0);
	else if (direction == DOWN)
		pos += pos_type(1, 0);
	else if (direction == LEFT)
		pos += pos_type(0, -1);
	else if (direction == RIGHT)
		pos += pos_type(0, 1);

	if(pos == point->getPos())
	{
		points.push_back(pos);

		return false;
	}

	return true;
}

bool Snake::checkCollide(const pos_type pos) const
{
	return std::find(points.cbegin(), points.cend(), pos) == points.cend();
}
