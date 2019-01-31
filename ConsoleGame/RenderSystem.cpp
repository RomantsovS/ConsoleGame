#include "tr_local.h"

RenderSystemLocal tr;
RenderSystem * renderSystem = &tr;

RenderSystemLocal::RenderSystemLocal(size_t ht, size_t wd, size_t bordHt, size_t bordWd, Screen::Pixel bordPix) :
	height(ht), width(wd), borderHeight(bordHt), borderWidth(bordWd), borderPixel(bordPix),
	screen(ht, wd, Screen::Pixel(' ', Screen::ConsoleColor::Black))
{
}

RenderSystemLocal::~RenderSystemLocal()
{
}

void RenderSystemLocal::init()
{

}

void RenderSystemLocal::draw()
{
	screen.display();
}

void RenderSystemLocal::clear()
{
	system("cls");
}
