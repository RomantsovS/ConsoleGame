#ifndef GUI_GUI_TEXTINSTANCE_H_
#define GUI_GUI_TEXTINSTANCE_H_

#include <string>

#include "../renderer/Screen.h"

class GUITextInstance {
public:
	GUITextInstance() :
		text(""),
		color(Screen::ConsoleColor::None),
		visible(false)
	{}
	~GUITextInstance() {}

	void Init();

	void SetText(const std::string& val) { text = val; }

	std::string text;
	Screen::ConsoleColor color;

	bool visible;
};

#endif
