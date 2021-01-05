#include "tr_local.h"
#include "../d3xp/Game_local.h"
#include "RenderWorld_local.h"
#include "ModelManager.h"
#include "../Sys/sys_public.h"

idRenderSystemLocal tr;
idRenderSystem * renderSystem = &tr;

/*
====================
RenderCommandBuffers
====================
*/
void idRenderSystemLocal::RenderCommandBuffers() {
	// r_skipBackEnd allows the entire time of the back end
	// to be removed from performance measurements, although
	// nothing will be drawn to the screen.  If the prints
	// are going to a file, or r_skipBackEnd is later disabled,
	// usefull data can be received.

	// r_skipRender is usually more usefull, because it will still
	// draw 2D graphics
	if (!r_skipBackEnd.GetBool()) {
		RB_ExecuteBackEndCommands();
	}
}

idRenderSystem::idRenderSystem() {
}

idRenderSystem::~idRenderSystem() {
}

idRenderSystemLocal::idRenderSystemLocal() {
	Clear();
}

idRenderSystemLocal::~idRenderSystemLocal() {
}

void idRenderSystemLocal::DrawString(Vector2 pos, const std::string& str, Screen::ConsoleColor color) {
	for (const auto& ch : str) {
		screen.set(pos, Screen::Pixel(ch, color));
		++pos.y;
	}
}

std::shared_ptr<idRenderWorld> idRenderSystemLocal::AllocRenderWorld() {
	auto rw = std::make_shared<idRenderWorldLocal>();
	worlds.push_back(rw);
	return std::dynamic_pointer_cast<idRenderWorld>(rw);
}

void idRenderSystemLocal::FreeRenderWorld(std::shared_ptr<idRenderWorld> rw) {
	auto iter = std::find(worlds.begin(), worlds.end(), rw);

	if (iter != worlds.end())
		worlds.erase(iter);
}

void idRenderSystemLocal::Display() {
	screen.display();
}

void idRenderSystemLocal::Clear() {
	frameCount = 0;
	viewCount = 0;

	screen.clear();
	worlds.clear();

	if (viewDef) {
		viewDef->renderWorld = nullptr;
		viewDef = nullptr;
	}

	update_frame = update_info = true;
}

void idRenderSystemLocal::FillBorder() {
	for (Screen::pos_type i = 0; i < height; ++i) {
		for (Screen::pos_type j = 0; j < borderWidth; ++j)
			screen.set(i, j, borderPixel);

		for (Screen::pos_type j = width - 1; j > width - 1 - borderWidth; --j)
			screen.set(i, j, borderPixel);
	}

	for (Screen::pos_type j = 0; j < width; ++j) {
		for (Screen::pos_type i = 0; i < borderHeight; ++i)
			screen.set(i, j, borderPixel);

		for (Screen::pos_type i = height - 1; i > height - 1 - borderHeight; --i)
			screen.set(i, j, borderPixel);
	}
}

void idRenderSystemLocal::ClearScreen() {
	screen.clear();
}
