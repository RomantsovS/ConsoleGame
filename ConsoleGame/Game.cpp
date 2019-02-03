#include <conio.h>
#include <iostream>

#include "Game.h"
#include "Point.h"
#include "Vector2.h"

RenderWorld *gameRenderWorld = NULL;

// the rest of the engine will only reference the "game" variable, while all local aspects stay hidden
Game gameLocal;

Game::Game()
{
}

Game::~Game()
{
	Destroy();
}

void Game::Init()
{
	idClass::Init();

	height = 20;
	width = 20;

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

	rand_eng.seed(static_cast<unsigned>(time(0)));

	renderSystem->Init();

	gameRenderWorld = new RenderWorldLocal();

	unsigned snakeSize = 3;

	/*std::default_random_engine rand_eng(static_cast<unsigned>(time(0) - 1));
	std::uniform_int_distribution<Screen::pos> u_h(0, height- snakeSize);
	std::uniform_int_distribution<Screen::pos> u_w(0, width);*/

	//Vector2 pos(u_h(rand_eng), u_w(rand_eng));

	//snake = std::make_shared<Snake>(snakeSize, pos.h, pos.w, Screen::Pixel(' ', Screen::Black), Screen::Pixel('*', Screen::Green));
	//addObject(snake->);

	AddRandomPoint();

	gameRunning = true;
}

void Game::Destroy()
{

}

void Game::AddObject(Entity *ent)
{
	entities.push_back(ent);
	
	gameRenderWorld->AddEntity(ent->getRenderEntity());
}

void Game::Frame()
{
	char c = 0;

	lastClock = clock();

	tr.Clear();

	if (_kbhit())
	{
		c = _getch();

		switch (c)
		{
		case 27:
			gameRunning = false;

			tr.Clear();

			std::cout << "enter Q to quit or any key to continue: ";

			std::cin >> c;

			while (std::cin.get() != '\n')
				continue;

			if (c == 'Q' || c == 'q')
			{
				gameRunning = false;

				return;
			}

			gameRunning = true;
		default:
			//onKeyPressed(c);
			;
		}
	}

	for (auto iter = entities.begin(); iter != entities.end(); ++iter)
	{
		(*iter)->Think();
	}

	try
	{
		gameRenderWorld->RenderScene();
	}
	catch (std::exception &err)
	{
		gameRunning = false;
		std::cout << err.what() << std::endl
			<< "press ane key to continue...\n";
		_getch();
	}
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

void Game::BreakTime()
{
	clock_t temp;
	temp = clock() + delayMilliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < temp)
	{
	}
}

void Game::AddRandomPoint()
{
	Vector2 pos(GetRandomValue(0U, height), GetRandomValue(0U, width));

	Dict args;

	args.Set("classname", "StaticEntity");
	args.Set("spawnclass", "StaticEntity");
	args.Set("pos", pos.ToString());
	
	SpawnEntityDef(args);

	size_t numIters = 0;

	/*while (!checkCollidePosToAllObjects(pos))
	{
		pos = { u_h(rand_eng), u_w(rand_eng) };

		if (++numIters == 10)
			break;
	}*/

	//auto point = new Point(pos, Screen::Pixel('*', GetRandomColor()));

	//AddObject(point);
}

void Game::RemoveInactiveObjects()
{
	for (auto iter = entities.begin(); iter != entities.end();)
	{
		if (!(*iter)->isActive())
		{
			entities.erase(iter);
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

	snake->think(*this);
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

Screen::ConsoleColor Game::GetRandomColor()
{
	std::uniform_int_distribution<int> u_c(0, colors.size() - 1);

	int col(u_c(rand_eng));

	return colors[col];
}

bool Game::SpawnEntityDef(const Dict & args)
{
	std::string className, spawn;
	idTypeInfo *cls;
	idClass *obj;

	spawnArgs = args;

	spawnArgs.GetString("classname", "", &className);

	spawnArgs.GetString("spawnclass", "", &spawn);

	if (!spawn.empty()) {

		cls = idClass::GetClass(spawn);
		if (!cls)
		{
			return false;
		}

		obj = cls->CreateInstance();
		if (!obj)
		{
			return false;
		}

		obj->CallSpawn();

		return true;
	}

	return false;
}

void Game::RegisterEntity(Entity * ent, int forceSpawnId, const Dict & spawnArgsToCopy)
{
	entities.push_back(ent);

	// Make a copy because TransferKeyValues clears the input parameter.
	Dict copiedArgs = spawnArgsToCopy;
	//ent->spawnArgs.TransferKeyValues(copiedArgs);

	ent->spawnArgs = copiedArgs;
}
