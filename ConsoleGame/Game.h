#ifndef Game_H
#define Game_H

#include <list>
#include <ctime>

//#include "Snake.h"
#include "RenderConsole.h"
#include "Entity.h"
#include "RenderWorld.h"

class Snake;

class Game
{
public:
	Game(size_t h, size_t w, size_t borderWd = 1, size_t borderHt = 1);

	~Game();

	void init();
	void destroy();

	void fillBorder(Screen &screen);

	void addObject(Entity *object);

	bool isGameRunning() const { return gameRunning; }

	size_t getBorderHeight() const { return borderHeight; }
	size_t getBorderWidth() const { return borderWidth; }

	size_t getUsedHeight() const { return height - 1 - borderHeight; }
	size_t getUsedWidth() const { return width - 1 - borderWidth; }

	void frame();

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/
private:
	void breakTime();

	void addRandomPoint();

	void removeInactiveObjects();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	Screen::ConsoleColor getRandomColor();

	//RenderSystem *renderSystem;
	RenderWorld *renderWorld;

	//std::shared_ptr<Snake> snake;
	std::list<Entity*> entityes;
	//std::vector<std::shared_ptr<EntityBase>> collideObjects;

	size_t height, width, borderWidth, borderHeight;

	clock_t lastClock;
	int delayMilliseconds;

	std::vector<Screen::ConsoleColor> colors;

	bool gameRunning;
};

#endif