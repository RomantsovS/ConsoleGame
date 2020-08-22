#include "tr_local.h"
#include "../d3xp/Game_local.h"
#include "RenderWorld_local.h"
#include "ModelManager.h"
#include "../Sys/sys_public.h"

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

void idRenderSystemLocal::Display()
{
	screen.display();
}

void idRenderSystemLocal::Clear()
{
	frameCount = 0;
	viewCount = 0;

	screen.clear();
	worlds.clear();

	if (viewDef)
	{
		viewDef->renderWorld = nullptr;
		viewDef = nullptr;
	}

	update_frame = update_info = true;
}

void idRenderSystemLocal::FillBorder()
{
	for (Screen::pos_type i = 0; i < height; ++i)
	{
		for (Screen::pos_type j = 0; j < borderWidth; ++j)
			screen.set(i, j, borderPixel);

		for (Screen::pos_type j = width - 1; j > width - 1 - borderWidth; --j)
			screen.set(i, j, borderPixel);
	}

	for (Screen::pos_type j = 0; j < width; ++j)
	{
		for (Screen::pos_type i = 0; i < borderHeight; ++i)
			screen.set(i, j, borderPixel);

		for (Screen::pos_type i = height - 1; i > height - 1 - borderHeight; --i)
			screen.set(i, j, borderPixel);
	}
}

void idRenderSystemLocal::ClearScreen()
{
	screen.clear();
}
