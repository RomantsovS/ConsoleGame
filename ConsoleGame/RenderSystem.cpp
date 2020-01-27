#include "tr_local.h"
#include "Game_local.h"
#include "RenderWorld_local.h"
#include "ModelManager.h"
#include "sys_public.h"

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
	screen.display(console);
	tr.updateFrame = false;
}

void idRenderSystemLocal::Clear()
{
	frameCount = 0;
	viewCount = 0;

	if(viewDef)
		viewDef->renderWorld = nullptr;

	screen.clear();
	worlds.clear();

	updateFrame = true;
	console.clear();
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

const int FPS_FRAMES = 6;

void idRenderSystemLocal::DrawFPS()
{
	static clock_t previousTimes[FPS_FRAMES];
	static int index;
	static clock_t previous;

	static clock_t prev_frame_update_time = Sys_Milliseconds();

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	auto t = Sys_Milliseconds();
	auto frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if (index > FPS_FRAMES) {
		// average multiple frames together to smooth changes out a bit
		int total = 0;
		for (int i = 0; i < FPS_FRAMES; i++) {
			total += previousTimes[i];
		}
		if (!total) {
			total = 1;
		}
		int fps = 1000000 * FPS_FRAMES / total;
		fps = (fps + 500) / 1000;

		console += std::to_string(fps) + " fps, " + std::to_string(total) + " total";
	}

	if (t - prev_frame_update_time > 65) {
		updateFrame = true;
		prev_frame_update_time = t;
	}
}
