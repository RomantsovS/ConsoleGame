#include "RenderConsole.h"

RenderConsole::RenderConsole(size_t ht, size_t wd, size_t bordHt, size_t bordWd, Screen::Pixel bordPix) :
	height(ht), width(wd), borderHeight(bordHt), borderWidth(bordWd), borderPixel(bordPix),
	screen(ht, wd, Screen::Pixel(' ', Screen::ConsoleColor::Black))
{
}

RenderConsole::~RenderConsole()
{
}

void RenderConsole::init()
{
	
}

void RenderConsole::update()
{
	screen.display();
}

void RenderConsole::clear()
{
	system("cls");
}
