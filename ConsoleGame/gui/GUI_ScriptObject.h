#ifndef GUI_GUI_SCRIPTOBJECT_H_
#define GUI_GUI_SCRIPTOBJECT_H_

#include <string>
#include <memory>

//#include "GUI_TextInstance.h"
//#include "GUI_ScriptVar.h"

//class GUIScriptVar;
class GUISpriteInstance;

/*
========================
An object in an action script is a collection of variables. functions are also variables.
========================
*/
class GUIScriptObject {
public:
	GUIScriptObject();
	~GUIScriptObject();

	void SetSprite(std::shared_ptr<GUISpriteInstance> s) { objectType = swfObjectType_t::SWF_OBJECT_SPRITE;  data.sprite = s; }
	std::shared_ptr<GUISpriteInstance> GetSprite() { return (objectType == swfObjectType_t::SWF_OBJECT_SPRITE) ? data.sprite : nullptr; }

	void SetText(std::shared_ptr<GUITextInstance> t) { objectType = swfObjectType_t::SWF_OBJECT_TEXT; data.text = t; }
	std::shared_ptr<GUITextInstance> GetText() { return data.text; }

	GUIScriptVar Get(const std::string& name);
	std::shared_ptr<GUISpriteInstance> GetSprite(const std::string& name);
	std::shared_ptr<GUITextInstance> GetText(const std::string& name);
	void Set(const std::string& name, const GUIScriptVar& value);

	std::shared_ptr<GUISpriteInstance> GetNestedSprite() { return data.sprite; }
private:
	struct guiNamedVar_t {
		guiNamedVar_t() : index(-1) { }
		~guiNamedVar_t() {}
		//guiNamedVar_t& operator=(const guiNamedVar_t& other);

		int index;
		std::string name;
		GUIScriptVar value;
	};
	std::vector<guiNamedVar_t> variables;

	enum class swfObjectType_t {
		SWF_OBJECT_OBJECT,
		SWF_OBJECT_ARRAY,
		SWF_OBJECT_SPRITE,
		SWF_OBJECT_TEXT
	} objectType;

	struct guiObjectData_t {
		std::shared_ptr<GUISpriteInstance> sprite;		// only valid if objectType == SWF_OBJECT_SPRITE
		std::shared_ptr<GUITextInstance> text;			// only valid if objectType == SWF_OBJECT_TEXT
	} data;

	guiNamedVar_t* GetVariable(const std::string& name, bool create);
};
#endif
