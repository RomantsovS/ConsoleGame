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
	screen.display(console);
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
	console.clear();
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

const int FPS_FRAMES = 8;
const size_t update_frame_time = 50000;

void idRenderSystemLocal::DrawFPS()
{
	static long long previousTimes[FPS_FRAMES];
	static int index;
	static long long previous;

	static auto prev_frame_update_time = Sys_Microseconds();
	static auto prev_info_update_time = prev_frame_update_time;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	auto t = Sys_Microseconds();
	auto frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;

	int fps = 0;

	if (index > FPS_FRAMES) {
		// average multiple frames together to smooth changes out a bit
		long long total = 0;
		for (int i = 0; i < FPS_FRAMES; i++) {
			total += previousTimes[i];
		}
		if (!total) {
			total = 1;
		}
		fps = static_cast<int>(1000000000ll * FPS_FRAMES / total);
		fps = (fps + 500) / 1000;
	}

	if (t - prev_frame_update_time > update_frame_time) {
		update_frame = true;
		prev_frame_update_time = t;
	}

	if (t - prev_info_update_time > update_frame_time) {
		update_info = true;
		prev_info_update_time = t;

		static char buf[256];

		sprintf_s(buf, " %8d fps, %8lld microsec last frame time", fps, frameTime);
		console.append(buf);
	}
}
