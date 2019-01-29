#ifndef Game_H
#define Game_H

#include <vector>
#include <ctime>

//#include "Snake.h"
#include "RenderConsole.h"
#include "EntityBase.h"

class Snake;

class Game
{
public:
	Game(size_t h, size_t w, size_t borderWd = 1, size_t borderHt = 1);

	~Game();

	void init();
	void destroy();

	void fillBorder(Screen &screen);

	void addObject(std::shared_ptr<EntityBase> object);

	bool isGameRunning() const { return gameRunning; }

	size_t getBorderHeight() const { return borderHeight; }
	size_t getBorderWidth() const { return borderWidth; }

	size_t getUsedHeight() const { return height - 1 - borderHeight; }
	size_t getUsedWidth() const { return width - 1 - borderWidth; }

	void frame();
	void update();

	/*void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);*/
private:
	void breakTime();

	void addRandomPoint();

	void removeInactiveObjects();

	/*void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);*/

	Screen::ConsoleColor getRandomColor();

	RenderSystem *renderSystem;

	//std::shared_ptr<Snake> snake;
	std::vector<std::shared_ptr<EntityBase>> objects;
	//std::vector<std::shared_ptr<EntityBase>> collideObjects;

	size_t height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;

	clock_t lastClock;
	int delayMilliseconds;

	std::vector<Screen::ConsoleColor> colors;

	bool gameRunning;
};

#endif