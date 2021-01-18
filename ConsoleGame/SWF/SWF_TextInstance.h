#ifndef GUI_GUI_TEXTINSTANCE_H_
#define GUI_GUI_TEXTINSTANCE_H_

#include <string>

#include "../renderer/Screen.h"

class idSWFTextInstance :public std::enable_shared_from_this<idSWFTextInstance> {
public:
	idSWFTextInstance() :
		swf(nullptr),
		scriptObject(std::make_shared<idSWFScriptObject>()),
		text(""),
		color(Screen::ConsoleColor::None),
		visible(false)
	{}
	~idSWFTextInstance() {}

	void Init(std::shared_ptr<idSWF> _gui);

	std::shared_ptr<idSWFScriptObject> GetScriptObject() { return scriptObject; }
	size_t GetTextLength();

	void SetText(const std::string& val) { text = val; }

	std::shared_ptr<idSWF> swf;

	// this text instance's script object
	std::shared_ptr<idSWFScriptObject> scriptObject;

	std::string text;
	Screen::ConsoleColor color;

	bool visible;
};

#endif
