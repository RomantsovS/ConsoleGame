#ifndef GUI_GUI_TEXTINSTANCE_H_
#define GUI_GUI_TEXTINSTANCE_H_

#include <string>

#include "../renderer/Screen.h"

class GUITextInstance :public std::enable_shared_from_this<GUITextInstance> {
public:
	GUITextInstance() :
		gui(nullptr),
		scriptObject(std::make_shared<GUIScriptObject>()),
		text(""),
		color(Screen::ConsoleColor::None),
		visible(false)
	{}
	~GUITextInstance() {}

	void Init(std::shared_ptr<GUI> _gui);

	std::shared_ptr<GUIScriptObject> GetScriptObject() { return scriptObject; }

	void SetText(const std::string& val) { text = val; }

	std::shared_ptr<GUI> gui;

	// this text instance's script object
	std::shared_ptr<GUIScriptObject> scriptObject;

	std::string text;
	Screen::ConsoleColor color;

	bool visible;
};

#endif
