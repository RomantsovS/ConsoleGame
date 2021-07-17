#ifndef GUI_GUI_TEXTINSTANCE_H_
#define GUI_GUI_TEXTINSTANCE_H_

#include <string>

#include "../renderer/Screen.h"

class idSWFTextInstance :public std::enable_shared_from_this<idSWFTextInstance> {
public:
	idSWFTextInstance() :
		scriptObject(std::make_shared<idSWFScriptObject>()),
		text(""),
		color(colorNone),
		visible(false) {
		swf.reset();
	}
	~idSWFTextInstance() {}

	void Init(std::shared_ptr<idSWF> _gui);

	std::shared_ptr<idSWFScriptObject> GetScriptObject() { return scriptObject; }
	size_t GetTextLength();

	void SetText(const std::string& val) { text = val; }

	void Clear();

	std::weak_ptr<idSWF> swf;

	// this text instance's script object
	std::shared_ptr<idSWFScriptObject> scriptObject;

	std::string text;
	int color;

	bool visible;
};

#endif
