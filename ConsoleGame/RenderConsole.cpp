#include "RenderConsole.h"

RenderConsole::RenderConsole(size_t ht, size_t wd)
{
}

RenderConsole::~RenderConsole()
{
}

void RenderConsole::addObject(std::shared_ptr<RenderEntity> object)
{
	objects.push_back(object);
}

void RenderConsole::init()
{
	screen = std::make_shared<Screen>(20, 50, Screen::Pixel(' ', Screen::ConsoleColor::Black));
}

void RenderConsole::update()
{
	screen->display();
}

void RenderConsole::clear()
{
	system("cls");
}
