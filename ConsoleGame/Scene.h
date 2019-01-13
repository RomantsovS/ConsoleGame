#ifndef SCENE
#define SCENE

#include <vector>

#include "Snake.h"
#include "Screen.h"

class Snake;

//enum directions { LEFT, RIGHT, UP, DOWN };

class Scene
{
	using pos = Screen::pos;
public:
	Scene(pos h, pos w, pos borderWd = 1, pos borderHt = 1, char bord = '#', int delay = 100);

	~Scene();

	void init();
	void destroy();

	void fillBorder(Screen &screen);

	void addObject(std::shared_ptr<SimpleObject> object);

	pos getBorderHeight() const { return borderHeight; }
	pos getBorderWidth() const { return borderWidth; }

	pos getUsedHeight() const { return height - 1 - borderHeight; }
	pos getUsedWidth() const { return width - 1 - borderWidth; }

	void MainLoop(Screen &screen);

	void drawToScreen(Screen &screen);

	void onEvent();
	void onKeyPressed(char c);
private:
	std::shared_ptr<Snake> snake;
	std::vector<std::shared_ptr<SimpleObject>> objects;
	std::vector<std::shared_ptr<SimpleObject>> collideObjects;

	pos height, width, borderWidth, borderHeight;
	char borderSymbol;

	int delayMilliseconds;

	void breakTime();

	void addRandomPoint();

	void removeObject(std::shared_ptr<SimpleObject> object);

	bool checkCollideObjects(std::shared_ptr<SimpleObject> object, SimpleObject::directions dir);

	void onMoveKeyPressed(SimpleObject::directions dir);

	bool checkCollidePosToAllObjects(pos_type pos);
};

#endif