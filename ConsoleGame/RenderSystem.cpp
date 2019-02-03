#include "tr_local.h"

RenderSystemLocal tr;
RenderSystem * renderSystem = &tr;

RenderSystem::RenderSystem()
{
}

RenderSystem::~RenderSystem()
{
}

RenderSystemLocal::RenderSystemLocal()
{
}

RenderSystemLocal::~RenderSystemLocal()
{
}

void RenderSystemLocal::Init()
{
	height = 20;
	width = 20;

	borderHeight = 1;
	borderWidth = 1;

	borderPixel = Screen::Pixel('#', Screen::ConsoleColor::White);

	screen = Screen(height, width, Screen::Pixel(' ', Screen::ConsoleColor::Black));
}

void RenderSystemLocal::Draw(const renderEntity_s &ent)
{
	
}

void RenderSystemLocal::Display()
{
	screen.display();
}

void RenderSystemLocal::Clear()
{
	system("cls");
	screen.clear();
}

void RenderSystemLocal::FillBorder()
{
	for (size_t i = 0; i < height; ++i)
	{
		for (size_t j = 0; j < borderWidth; ++j)
			screen.set(i, j, borderPixel);

		for (size_t j = width - 1; j > width - 1 - borderWidth; --j)
			screen.set(i, j, borderPixel);
	}

	for (size_t j = 0; j < width; ++j)
	{
		for (size_t i = 0; i < borderHeight; ++i)
			screen.set(i, j, borderPixel);

		for (size_t i = height - 1; i > height - 1 - borderHeight; --i)
			screen.set(i, j, borderPixel);
	}
}