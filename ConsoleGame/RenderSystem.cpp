#include "tr_local.h"
#include "Game_local.h"
#include "RenderWorld_local.h"
#include "ModelManager.h"

idRenderSystemLocal tr;
idRenderSystem * renderSystem = &tr;

idRenderSystem::idRenderSystem()
{
}

idRenderSystem::~idRenderSystem()
{
}

idRenderSystemLocal::idRenderSystemLocal()
{
	Clear();
}

idRenderSystemLocal::~idRenderSystemLocal()
{
}

std::shared_ptr<idRenderWorld> idRenderSystemLocal::AllocRenderWorld()
{
	auto rw = std::make_shared<idRenderWorldLocal>();
	worlds.push_back(rw);
	return std::dynamic_pointer_cast<idRenderWorld>(rw);
}

void idRenderSystemLocal::FreeRenderWorld(std::shared_ptr<idRenderWorld> rw)
{
	auto iter = std::find(worlds.begin(), worlds.end(), rw);

	if (iter != worlds.end())
		worlds.erase(iter);
}

void idRenderSystemLocal::Draw(const renderEntity_t &ent)
{
	/*auto pixels = ent.hModel->

	for (auto pixelIter = pixels.cbegin(); pixelIter != pixels.cend(); ++pixelIter)
	{
		screen.set(Vector2(borderHeight, borderWidth) + ent.origin + pixelIter->origin, pixelIter->screenPixel);
	}*/
}

void idRenderSystemLocal::Display()
{
	screen.display();
}

void idRenderSystemLocal::Clear()
{
	frameCount = 0;
	viewCount = 0;

	if(viewDef)
		viewDef->renderWorld = nullptr;

	screen.clear();
	worlds.clear();
}

void idRenderSystemLocal::FillBorder()
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

void idRenderSystemLocal::ClearScreen()
{
	system("cls");

	screen.clear();
}
