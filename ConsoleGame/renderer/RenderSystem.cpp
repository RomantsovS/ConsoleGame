#pragma hdrstop
#include "../idlib/precompiled.h"

#include "tr_local.h"

idRenderSystemLocal tr;
idRenderSystem * renderSystem = &tr;

constexpr int BIGCHAR_WIDTH = 8;
constexpr int BIGCHAR_HEIGHT = 8;
 
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

	if (!tr.update_frame)
		return;
	
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

idRenderSystemLocal::idRenderSystemLocal() : update_frame_time(100) {
	Clear();
}

idRenderSystemLocal::~idRenderSystemLocal() {
}

/*
=============
idRenderSystemLocal::DrawStretchPic
=============
*/
void idRenderSystemLocal::DrawStretchPic(int x, int y, int w, int h, int s1, int t1, const idMaterial* material) {
	if (!R_IsInitialized()) {
		return;
	}
	if (material == nullptr) {
		return;
	}

	const auto& imagePixels = material->GetStage()->image->GetPixels();

	for(int i = 0; i < h; ++i) {
		for(int j = 0; j < w; ++j) {
			int pixelIndex = t1 * BIGCHAR_WIDTH * BIGCHAR_WIDTH * 16 + i * BIGCHAR_WIDTH * 16 + s1 * BIGCHAR_WIDTH + j;
			tr.screen.set(x + j, y + i, imagePixels[pixelIndex].screenPixel);
		}
	}
}

/*
=====================
idRenderSystemLocal::DrawBigChar
=====================
*/
void idRenderSystemLocal::DrawBigChar(int x, int y, int ch) {
	int row{}, col{};

	ch &= 255;

	if (ch == ' ') {
		return;
	}

	if (y < -BIGCHAR_HEIGHT) {
		return;
	}

	col = ch % 16;
	row = (ch - ' ') / 16;

	DrawStretchPic(x, y, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, col, row, charSetMaterial.get());
}

/*
==================
idRenderSystemLocal::DrawBigStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void idRenderSystemLocal::DrawBigStringExt(int x, int y, const std::string& string, const int setColor, bool forceColor) {
	//idVec4		color;
	int			xx;

	// draw the colored text
	xx = x;
	//SetColor(setColor);
	for(const auto& s : string) {
		/*if (idStr::IsColor(s)) {
			if (!forceColor) {
				if (*(s + 1) == C_COLOR_DEFAULT) {
					SetColor(setColor);
				}
				else {
					color = idStr::ColorForIndex(*(s + 1));
					color[3] = setColor[3];
					SetColor(color);
				}
			}
			s += 2;
			continue;
		}*/
		DrawBigChar(xx, y, s);
		xx += BIGCHAR_WIDTH;
	}
	//SetColor(colorWhite);
}

void idRenderSystemLocal::DrawPositionedString(Vector2 pos, const std::string& str, int color) {
	for (const auto& ch : str) {
		screen.set(pos, Screen::Pixel(ch, color));
		++pos.y;
	}
}

void idRenderSystemLocal::DrawString(const std::string& text, const int color) {
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

void idRenderSystemLocal::UpdateTimers() {
	static auto prev_frame_update_time = Sys_Milliseconds();

	auto t = Sys_Milliseconds();

	if (t - prev_frame_update_time > update_frame_time) {
		tr.update_frame = true;
		prev_frame_update_time = t;
	}

	if (!tr.update_frame)
		return;

	tr.ClearScreen();
	tr.FillBorder();
}
