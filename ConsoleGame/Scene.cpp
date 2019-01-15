#include <random>
#include <conio.h>
#include <iostream>

#include "Scene.h"

Scene::Scene(pos h, pos w, pos borderWd, pos borderHt, Screen::Pixel bord, int delay) : height(h), width(w),
					borderHeight(borderHt), borderWidth(borderWd), borderPixel(bord), lastClock(0), delayMilliseconds(delay)
{
	
}

Scene::~Scene()
{
	destroy();
}

void Scene::init()
{
	static std::default_random_engine rand_eng;
	static std::uniform_int_distribution<Screen::pos> u_h(getBorderHeight(), getUsedHeight());
	static std::uniform_int_distribution<Screen::pos> u_w(getBorderWidth(), getUsedWidth());

	pos_type pos(u_h(rand_eng), u_w(rand_eng));

	snake = std::make_shared<Snake>(3, pos.h, pos.w, Screen::Pixel(' ', Screen::Black), Screen::Pixel('*', Screen::Green));
	addObject(std::static_pointer_cast<SimpleObject>(snake));

	addRandomPoint();
}

void Scene::destroy()
{

}

void Scene::fillBorder(Screen &screen)
{
	for (pos i = 0; i < height; ++i)
	{
		for (pos j = 0; j < borderWidth; ++j)
			screen.set(i, j, borderPixel);

		for (pos j = width - 1; j > width - 1 - borderWidth; --j)
			screen.set(i, j, borderPixel);
	}

	for (pos j = 0; j < width; ++j)
	{
		for (pos i = 0; i < borderHeight; ++i)
			screen.set(i, j, borderPixel);

		for (pos i = height - 1; i > height - 1 - borderHeight; --i)
			screen.set(i, j, borderPixel);
	}
}

void Scene::addObject(std::shared_ptr<SimpleObject> object)
{
	objects.push_back(object);
	collideObjects.push_back(object);
}

void Scene::MainLoop(Screen &screen)
{
	char c = 0;

	bool gameRunning = true;
	lastClock = clock();

	while (gameRunning)
	{
		system("cls");

		if (_kbhit())
		{
			c = _getch();

			switch (c)
			{
			case 27:
				gameRunning = false;

				system("cls");

				std::cout << "enter Q to quit or any key to continue: ";

				std::cin >> c;

				if (c == 'Q' || c == 'q')
					break;

				gameRunning = true;
			default:
				onKeyPressed(c);
				;
			}
		}

		auto tempClock = clock();

		if (tempClock - lastClock >= delayMilliseconds * CLOCKS_PER_SEC / 1000)
		{
			onEvent();
			lastClock = tempClock;
		}

		screen.clear();
		fillBorder(screen);

		try
		{
			drawToScreen(screen);
		}
		catch (std::exception &err)
		{
			std::cout << err.what() << std::endl
				<< "press ane key to continue...\n";
			_getch();
		}

		screen.display();
	}
}

void Scene::drawToScreen(Screen & screen)
{
	for (auto iter = objects.begin(); iter != objects.end(); ++iter)
	{
		auto obj = *iter;

		obj->drawToScreen(screen);
	}
}

void Scene::onEvent()
{	
	if (snake && snake->checkCollide(*this))
		if (checkCollideObjects(snake))
			snake->move();
}

void Scene::onKeyPressed(char c)
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
}

void Scene::breakTime()
{
	clock_t temp;
	temp = clock() + delayMilliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < temp)
	{
	}
}

void Scene::addRandomPoint()
{
	static std::default_random_engine rand_eng;
	static std::uniform_int_distribution<Screen::pos> u_h(getBorderHeight(), getUsedHeight());
	static std::uniform_int_distribution<Screen::pos> u_w(getBorderWidth(), getUsedWidth());

	pos_type pos(u_h(rand_eng), u_w(rand_eng));

	size_t numIters = 0;

	while (!checkCollidePosToAllObjects(pos))
	{
		pos = { u_h(rand_eng), u_w(rand_eng) };

		if (++numIters == 10)
			break;
	}

	static std::uniform_int_distribution<int> u_c(static_cast<int>(Screen::Black), static_cast<int>(Screen::Yellow));

	int col(u_c(rand_eng));

	auto point = std::make_shared<Point>(pos, Screen::Pixel('*', static_cast<Screen::ConsoleColor>(col)));

	addObject(point);
}

void Scene::removeObject(std::shared_ptr<SimpleObject> object)
{
	for (auto iter = collideObjects.begin(); iter != collideObjects.end();)
	{
		if (*iter == object)
		{
			iter = collideObjects.erase(iter);
			break;
		}
		else
			++iter;
	}

	for (auto iter = objects.begin(); iter != objects.end();)
	{
		if (*iter == object)
		{
			objects.erase(iter);
			break;
		}
		else
			++iter;
	}
}

bool Scene::checkCollideObjects(std::shared_ptr<SimpleObject> object)
{
	for (auto iter = collideObjects.begin(); iter != collideObjects.end();)
	{
		auto obj = *iter;

		if (object == obj)
		{
			++iter;
			continue;
		}
			

		if (!object->checkCollide(std::static_pointer_cast<Point>(obj)))
		{
			removeObject(obj);

			addRandomPoint();
			addRandomPoint();

			delayMilliseconds -= 10;

			return false;
		}
		else
			++iter;
	}

	return true;
}

void Scene::onMoveKeyPressed(SimpleObject::directions dir)
{
	snake->setDirection(dir);

	if (snake->checkCollide(*this))
		if (checkCollideObjects(snake))
			snake->move();
}

bool Scene::checkCollidePosToAllObjects(pos_type pos)
{
	for (auto iter = collideObjects.cbegin(); iter != collideObjects.cend(); ++iter)
	{
		auto obj = *iter;

		if (!obj->checkCollide(pos))
			return false;
	}

	return true;
}
