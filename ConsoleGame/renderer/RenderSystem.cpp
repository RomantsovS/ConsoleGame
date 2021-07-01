#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"

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

void idRenderSystemLocal::DrawPositionedString(Vector2 pos, const std::string& str, Screen::ConsoleColor color) {
	for (const auto& ch : str) {
		screen.set(pos, Screen::Pixel(ch, color));
		++pos.y;
	}
}

void idRenderSystemLocal::DrawString(const std::string& text, const Screen::ConsoleColor& color) {
	if (!text.empty()) {
		tr.screen.writeInColor(text, color);
	}
}

std::shared_ptr<idRenderWorld> idRenderSystemLocal::AllocRenderWorld() {
#ifdef DEBUG
	auto rw = std::shared_ptr<idRenderWorldLocal>(DBG_NEW idRenderWorldLocal());
#else
	auto rw = std::make_shared<idRenderWorldLocal>();
#endif
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
	
	// free all the entityDefs, lightDefs, portals, etc
	for(auto &world : worlds)
		world->FreeWorld();

	worlds.clear();

	if (viewDef) {
		viewDef->renderWorld = nullptr;
		viewDef = nullptr;
	}

	update_frame = update_info = true;
}

void idRenderSystemLocal::FillBorder() {
	for (Screen::pos_type y = 0; y < height; ++y) {
		for (Screen::pos_type x = 0; x < borderWidth; ++x)
			screen.set(x, y, borderPixel);

		for (Screen::pos_type x = width - 1; x > width - 1 - borderWidth; --x)
			screen.set(x, y, borderPixel);
	}

	for (Screen::pos_type x = 0; x < width; ++x) {
		for (Screen::pos_type y = 0; y < borderHeight; ++y)
			screen.set(x, y, borderPixel);

		for (Screen::pos_type y = height - 1; y > height - 1 - borderHeight; --y)
			screen.set(x, y, borderPixel);
	}
}

void idRenderSystemLocal::ClearScreen() {
	screen.clear();
}
