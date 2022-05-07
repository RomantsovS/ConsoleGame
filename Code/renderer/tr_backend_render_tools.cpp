#include "idlib/precompiled.h"


#include "tr_local.h"
#include "../d3xp/Game_local.h"

constexpr size_t MAX_DEBUG_LINES = 512;

struct debugLine_t {
	int			rgb;
	Vector2		start;
	Vector2		end;
	bool		depthTest;
	int			lifeTime;
};

debugLine_t		rb_debugLines[MAX_DEBUG_LINES];
int				rb_numDebugLines = 0;
int				rb_debugLineTime = 0;

constexpr size_t MAX_DEBUG_TEXT = 10;
idCVar max_debug_text("max_debug_text", "4", CVAR_TOOL, "", 0, MAX_DEBUG_TEXT);

struct debugText_t {
	std::string text;
	Vector2 origin;
	Screen::color_type color;
	int lifeTime;
};

std::vector<debugText_t> rb_debugText(MAX_DEBUG_TEXT);
int rb_numDebugText = 0;
int rb_debugTextTime = 0;

/*
================
RB_ClearDebugText
================
*/
void RB_ClearDebugText(int time) {
	int			i{ 0 };
	int			num;

	rb_debugTextTime = time;

	if (!time) {
		// free up our strings
		for (auto& text : rb_debugText) {
			text.text.clear();
		}
		rb_numDebugText = 0;
		return;
	}

	// copy any text that still needs to be drawn
	num = 0;

	for(auto& text : rb_debugText) {
		if (text.lifeTime > time) {
			if (num != i) {
				rb_debugText[num] = text;
			}
			num++;
		}
		++i;
	}
	rb_numDebugText = num;
}

/*
================
RB_AddDebugText
================
*/
void RB_AddDebugText(std::string text, const Vector2 &origin, const Screen::color_type color, const int lifetime) {
	if (rb_numDebugText < max_debug_text.GetInteger()) {
		auto debugText = &rb_debugText.at(rb_numDebugText++);
		debugText->text = std::move(text);
		debugText->origin = origin;
		debugText->color = color;
		debugText->lifeTime = rb_debugTextTime + lifetime;
	}
}

/*
================
RB_DrawText

  oriented on the viewaxis>	ConsoleGame.exe!RB_DrawText(const std::string & text, const Vector2 & origin, const Screen::ConsoleColor & color) Line 93	C++

  align can be 0-left, 1-center (default), 2-right
================
*/
/*
void RB_DrawText(const std::string &text, const Vector2 &origin, const Screen::color_type color) {
	if (!text.empty()) {
		tr.screen.writeInColor(text, color);
	}
}*/

/*
================
RB_ShowDebugText
================
*/
void RB_ShowDebugText() {
	int i{ 0 };

	if (!rb_numDebugText) {
		return;
	}

	for (auto& text : rb_debugText) {
		renderSystem->DrawBigStringExt(0, r_console_pos.GetInteger() + (i + 1) * 10, text.text, text.color, true);
		++i;
	}
}

/*
================
RB_ClearDebugLines
================
*/
void RB_ClearDebugLines(int time) noexcept {
	int			i;
	int			num;
	debugLine_t* line;

	rb_debugLineTime = time;

	if (!time) {
		rb_numDebugLines = 0;
		return;
	}

	// copy any lines that still need to be drawn
	num = 0;
	line = rb_debugLines;
	for (i = 0; i < rb_numDebugLines; i++, line++) {
		if (line->lifeTime > time) {
			if (num != i) {
				rb_debugLines[num] = *line;
			}
			num++;
		}
	}
	rb_numDebugLines = num;
}

void RB_AddDebugLine(const Screen::color_type color, const Vector2& start, const Vector2& end, const int lifeTime, const bool depthTest) noexcept {
	debugLine_t* line;

	if (rb_numDebugLines < MAX_DEBUG_LINES) {
		line = &rb_debugLines[rb_numDebugLines++];
		line->rgb = color;
		line->start = start;
		line->end = end;
		line->depthTest = depthTest;
		line->lifeTime = rb_debugLineTime + lifeTime;
	}
}

/*
================
RB_ShowDebugLines
================
*/
void RB_ShowDebugLines() {
	int			i;
	debugLine_t* line;
	Screen::pos_type x_pos, y_pos;

	if (!rb_numDebugLines) {
		return;
	}

	// all lines are expressed in world coordinates
	constexpr char debug_symbol = '^';

	line = rb_debugLines;
	for (i = 0; i < rb_numDebugLines; i++, line++) {
		if (!line->depthTest) {
			for (x_pos = static_cast<Screen::pos_type>(line->start.x); x_pos <= line->end.x; ++x_pos) {
				tr.screen.set(x_pos/* + tr.borderHeight*/, static_cast<Screen::pos_type>(line->start.y)/* + tr.borderWidth*/, Screen::Pixel(debug_symbol, line->rgb));
			}
			
			for (y_pos = static_cast<Screen::pos_type>(line->start.y); y_pos <= line->end.y; ++y_pos) {
				tr.screen.set(static_cast<Screen::pos_type>(line->start.x) /* + tr.borderHeight*/, y_pos/* + tr.borderWidth*/, Screen::Pixel(debug_symbol, line->rgb));
			}
		}
	}
}

/*
=================
RB_RenderDebugTools
=================
*/

void RB_RenderDebugTools() {
	RB_ShowDebugLines();
	//RB_ShowDebugText();
}