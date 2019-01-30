#include <random>
#include <conio.h>
#include <iostream>

#include "Game.h"
#include "Point.h"
#include "Vector2.h"

Game::Game(size_t h, size_t w, size_t borderWd, size_t borderHt)
{
}

Game::~Game()
{
	destroy();
}

void Game::init()
{
	delayMilliseconds = 100;

	colors.push_back(Screen::Green);
	colors.push_back(Screen::Cyan);
	colors.push_back(Screen::Red);
	colors.push_back(Screen::Magenta);
	colors.push_back(Screen::Brown);
	colors.push_back(Screen::LightGray);
	colors.push_back(Screen::LightGreen);
	colors.push_back(Screen::LightCyan);
	colors.push_back(Screen::LightRed);
	colors.push_back(Screen::LightMagenta);
	colors.push_back(Screen::Yellow);
	colors.push_back(Screen::White);

	renderSystem = new RenderConsole(height, width, borderHeight, borderWidth);
	renderSystem->init();

	unsigned snakeSize = 3;

	std::default_random_engine rand_eng(static_cast<unsigned>(time(0) - 1));
	std::uniform_int_distribution<Screen::pos> u_h(getBorderHeight(), getUsedHeight() - snakeSize);
	std::uniform_int_distribution<Screen::pos> u_w(getBorderWidth(), getUsedWidth());

	//Vector2 pos(u_h(rand_eng), u_w(rand_eng));

	//snake = std::make_shared<Snake>(snakeSize, pos.h, pos.w, Screen::Pixel(' ', Screen::Black), Screen::Pixel('*', Screen::Green));
	//addObject(snake->);

	addRandomPoint();
}

void Game::destroy()
{

}

void Game::fillBorder(Screen &screen)
{
	/*for (size_t i = 0; i < height; ++i)
	{
		for (size_t j = 0; j < borderWidth; ++j)
			screen.set(i, j, borderPixel);

		for (size_t j = width - 1; j > width - 1 - borderWidth; --j)
			screen.set(i, j, borderPixel);
	}

	for (size_t j = 0; j < width; ++j)
	{
		for (size_t i = 0; i < borderHeight; ++i)
			screen.set(i, j, borderPixel);

		for (size_t i = height - 1; i > height - 1 - borderHeight; --i)
			screen.set(i, j, borderPixel);
	}*/
}

void Game::addObject(Entity *object)
{
	renderWorld->addEntity(object->getRenderEntity());

	//collideObjects.push_back(object);
}

void Game::frame()
{
	char c = 0;

	lastClock = clock();

	renderSystem->clear();

	if (_kbhit())
	{
		c = _getch();

		switch (c)
		{
		case 27:
			gameRunning = false;

			renderSystem->clear();

			std::cout << "enter Q to quit or any key to continue: ";

			std::cin >> c;

			if (c == 'Q' || c == 'q')
				break;

			gameRunning = true;
		default:
			//onKeyPressed(c);
			;
		}
	}

	auto tempClock = clock();

	if (tempClock - lastClock >= delayMilliseconds * CLOCKS_PER_SEC / 1000)
	{
		update();
		lastClock = tempClock;
	}

	renderSystem->clear();

	//fillBorder(screen);

	try
	{
		renderSystem->update();
	}
	catch (std::exception &err)
	{
		std::cout << err.what() << std::endl
			<< "press ane key to continue...\n";
		_getch();
	}
}

void Game::update()
{
	/*auto objectsCopy = objects;

	for (auto iter = objectsCopy.begin(); iter != objectsCopy.end(); ++iter)
	{
		(*iter)->update(*this);
	}

	removeInactiveObjects();*/
}

/*
void Game::onKeyPressed(char c)
{
	switch (c)
	{
	case 72: case 'w':
	{
		onMoveKeyPressed(SimpleObject::UP);

		break;
	}
	case 80: case 's':
	{
		onMoveKeyPressed(SimpleObject::DOWN);

		break;
	}
	case 75: case 'a':
	{
		onMoveKeyPressed(SimpleObject::LEFT);

		break;
	}
	case 77: case 'd':
	{
		onMoveKeyPressed(SimpleObject::RIGHT);

		break;
	}
	}
}*/

void Game::breakTime()
{
	clock_t temp;
	temp = clock() + delayMilliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < temp)
	{
	}
}

void Game::addRandomPoint()
{
	static std::default_random_engine rand_eng(static_cast<unsigned>(time(0)));
	static std::uniform_int_distribution<Screen::pos> u_h(getBorderHeight(), getUsedHeight());
	static std::uniform_int_distribution<Screen::pos> u_w(getBorderWidth(), getUsedWidth());

	Vector2 pos(u_h(rand_eng), u_w(rand_eng));

	size_t numIters = 0;

	/*while (!checkCollidePosToAllObjects(pos))
	{
		pos = { u_h(rand_eng), u_w(rand_eng) };

		if (++numIters == 10)
			break;
	}*/

	auto point = new Point(pos, Screen::Pixel('*', getRandomColor()));

	addObject(point);
}

void Game::removeInactiveObjects()
{
	for (auto iter = objects.begin(); iter != objects.end();)
	{
		if (!(*iter)->isActive())
		{
			objects.erase(iter);
			break;
		}
		else
			++iter;
	}
}

/*
bool Game::checkCollideObjects(SimpleObject *object)
{
	for (auto iter = collideObjects.begin(); iter != collideObjects.end();)
	{
		auto obj = *iter;

		if (object == obj.get())
		{
			++iter;
			continue;
		}
			

		if (!object->checkCollide(std::static_pointer_cast<Point>(obj)))
		{
			obj->setActive(false);

			addRandomPoint();
			addRandomPoint();

			delayMilliseconds -= 10;

			return false;
		}
		else
			++iter;
	}

	return true;
}*/

/*
void Game::onMoveKeyPressed(SimpleObject::directions dir)
{
	snake->setDirection(dir);

	snake->update(*this);
}*/

/*
bool Game::checkCollidePosToAllObjects(pos_type pos)
{
	for (auto iter = collideObjects.cbegin(); iter != collideObjects.cend(); ++iter)
	{
		auto obj = *iter;

		if (!obj->checkCollide(pos))
			return false;
	}

	return true;
}*/

Screen::ConsoleColor Game::getRandomColor()
{
	static std::default_random_engine rand_eng(static_cast<unsigned>(time(0)));

	static std::uniform_int_distribution<int> u_c(0, colors.size() - 1);

	int col(u_c(rand_eng));

	return colors[col];
}