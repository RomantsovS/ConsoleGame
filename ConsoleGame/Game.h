#ifndef Game_H
#define Game_H

#include <vector>
#include <ctime>

#include "Snake.h"
#include "Screen.h"
#include "RenderSystem.h"
#include "Entity.h"

class Snake;

class Game
{
	using pos = Screen::pos;
public:
	Game(pos h, pos w, pos borderWd = 1, pos borderHt = 1, Screen::Pixel bord = Screen::Pixel('#', Screen::White), int delay = 100);

	~Game();

	void init();
	void destroy();

	void fillBorder(Screen &screen);

	void addObject(std::shared_ptr<Entity> object);

	pos getBorderHeight() const { return borderHeight; }
	pos getBorderWidth() const { return borderWidth; }

	pos getUsedHeight() const { return height - 1 - borderHeight; }
	pos getUsedWidth() const { return width - 1 - borderWidth; }

	void MainLoop(Screen &screen);
	void update();
	void drawToScreen(Screen &screen);

	void onKeyPressed(char c);

	bool checkCollideObjects(SimpleObject *object);
private:
	void breakTime();

	void addRandomPoint();

	void removeInactiveObjects();

	void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);

	Screen::ConsoleColor getRandomColor();

	RenderSystem *renderSystem;

	std::shared_ptr<Snake> snake;
	std::vector<std::shared_ptr<SimpleObject>> collideObjects;

	pos height, width, borderWidth, borderHeight;
	Screen::Pixel borderPixel;

	clock_t lastClock;
	int delayMilliseconds;

	std::vector<Screen::ConsoleColor> colors;
};

#endif