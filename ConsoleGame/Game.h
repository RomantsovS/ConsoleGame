#ifndef Game_H
#define Game_H

#include <list>
#include <ctime>
#include <random>

#include "Entity.h"
#include "tr_local.h"

class Snake;

class Game
{
public:
	Game();

	~Game();

	void init();
	void destroy();

	void addObject(Entity *object);

	bool isGameRunning() const { return gameRunning; }

	void frame();

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/

	template <typename T>
	T getRandomValue(T min, T max);

	Screen::ConsoleColor getRandomColor();
private:
	void breakTime();

	void addRandomPoint();

	void removeInactiveObjects();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	//RenderSystem *renderSystem;
	RenderWorld *renderWorld;

	//std::shared_ptr<Snake> snake;
	std::list<Entity*> entities;
	//std::vector<std::shared_ptr<EntityBase>> collideObjects;

	size_t height, width;

	clock_t lastClock;
	int delayMilliseconds;

	std::vector<Screen::ConsoleColor> colors;

	bool gameRunning;

	std::default_random_engine rand_eng;
};

#endif

template<typename T>
inline T Game::getRandomValue(T min, T max)
{
	std::uniform_int_distribution<Screen::pos> u(min, max);
	
	return u(rand_eng);
}
