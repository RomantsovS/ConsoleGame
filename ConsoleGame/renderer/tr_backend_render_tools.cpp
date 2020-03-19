#include "tr_local.h"

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

  oriented on the viewaxis
  align can be 0-left, 1-center (default), 2-right
================
*/
static void RB_DrawText(const std::string &text, const Vector2 &origin, const Screen::ConsoleColor &color) {
	if (!text.empty()) {
		SetColor(color, tr.screen.getBackgroundPixel().color);
		std::cout << text << std::endl;
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
=================
RB_RenderDebugTools
=================
*/
void RB_RenderDebugTools() {
	RB_ShowDebugText();
}