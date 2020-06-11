#include "tr_local.h"

#define MAX_DEBUG_LINES			256

struct debugLine_t {
	Screen::ConsoleColor		rgb;
	Vector2		start;
	Vector2		end;
	bool		depthTest;
	int			lifeTime;
};

debugLine_t		rb_debugLines[MAX_DEBUG_LINES];
int				rb_numDebugLines = 0;
int				rb_debugLineTime = 0;

const size_t MAX_DEBUG_TEXT = 8;

struct debugText_t {
	std::string text;
	Vector2 origin;
	Screen::ConsoleColor color;
	int lifeTime;
};

debugText_t rb_debugText[MAX_DEBUG_TEXT];
int rb_numDebugText = 0;
int rb_debugTextTime = 0;

/*
================
RB_ClearDebugText
================
*/
void RB_ClearDebugText(int time) {
	int			i;
	int			num;
	debugText_t	*text;

	rb_debugTextTime = time;

	if (!time) {
		// free up our strings
		text = rb_debugText;
		for (i = 0; i < MAX_DEBUG_TEXT; i++, text++) {
			text->text.clear();
		}
		rb_numDebugText = 0;
		return;
	}

	// copy any text that still needs to be drawn
	num = 0;
	text = rb_debugText;
	for (i = 0; i < rb_numDebugText; i++, text++) {
		if (text->lifeTime > time) {
			if (num != i) {
				rb_debugText[num] = *text;
			}
			num++;
		}
	}
	rb_numDebugText = num;
}

/*
================
RB_AddDebugText
================
*/
void RB_AddDebugText(const std::string &text, const Vector2 &origin, const Screen::ConsoleColor &color, const int lifetime) {
	if (rb_numDebugText < MAX_DEBUG_TEXT) {
		auto debugText = &rb_debugText[rb_numDebugText++];
		debugText->text = text;
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
static void RB_DrawText(const std::string &text, const Vector2 &origin, const Screen::ConsoleColor &color) {
	if (!text.empty()) {
		tr.screen.writeInColor(text, color);
	}
}

/*
================
RB_ShowDebugText
================
*/
void RB_ShowDebugText() {
	int			i;
	debugText_t	*text;

	if (!rb_numDebugText) {
		return;
	}

	text = rb_debugText;
	for (i = 0; i < rb_numDebugText; i++, text++) {
		RB_DrawText(text->text, text->origin, text->color);
	}
}

/*
================
RB_ClearDebugLines
================
*/
void RB_ClearDebugLines(int time) {
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

void RB_AddDebugLine(const Screen::ConsoleColor color, const Vector2& start, const Vector2& end, const int lifeTime, const bool depthTest)
{
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
	float x_pos, y_pos;

	if (!rb_numDebugLines) {
		return;
	}

	// all lines are expressed in world coordinates
	char debug_symbol = '^';

	line = rb_debugLines;
	for (i = 0; i < rb_numDebugLines; i++, line++) {
		if (!line->depthTest) {
			for (x_pos = static_cast<Screen::pos_type>(line->start.x); x_pos <= line->end.x; ++x_pos)
			{
				tr.screen.set(static_cast<Screen::pos_type>(x_pos) + tr.borderHeight, static_cast<Screen::pos_type>(line->start.y) + tr.borderWidth, Screen::Pixel(debug_symbol, line->rgb));
			}
			
			for (y_pos = static_cast<Screen::pos_type>(line->start.y); y_pos <= line->end.y; ++y_pos)
			{
				tr.screen.set(static_cast<Screen::pos_type>(line->start.x) + tr.borderHeight, static_cast<Screen::pos_type>(y_pos) + tr.borderWidth, Screen::Pixel(debug_symbol, line->rgb));
			}
		}
	}
}

/*
=================
RB_RenderDebugTools
=================
*/
void RB_RenderDebugToolsBefore() {
	if (!tr.update_frame)
		return;

	tr.ClearScreen();
	tr.FillBorder();

	RB_ShowDebugLines();
}

void RB_RenderDebugTools() {
	RB_ShowDebugText();
}