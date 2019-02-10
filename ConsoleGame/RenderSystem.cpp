#include "tr_local.h"
#include "Game_local.h"

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
	borderHeight = 1;
	borderWidth = 1;

	height = gameLocal.GetHeight() + borderHeight * 2;
	width = gameLocal.GetWidth() + borderWidth * 2;

	borderPixel = Screen::Pixel('#', Screen::ConsoleColor::White);

	screen = Screen(height, width, Screen::Pixel(' ', Screen::ConsoleColor::Black));
}

void RenderSystemLocal::Draw(const renderEntity_s &ent)
{
	auto pixels = ent.hModel->GetJoints();

	for (auto pixelIter = pixels.cbegin(); pixelIter != pixels.cend(); ++pixelIter)
	{
		screen.set(Vector2(borderHeight, borderWidth) + ent.origin + pixelIter->origin, pixelIter->screenPixel);
	}
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