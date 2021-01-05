#ifndef GUI_GUI_SCRIPTOBJECT_H_
#define GUI_GUI_SCRIPTOBJECT_H_

#include <string>
#include <memory>

#include "GUI_TextInstance.h"
#include "GUI_ScriptVar.h"

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

	void SetSprite(std::shared_ptr<GUISpriteInstance> s) { data.sprite = s; }

	std::shared_ptr<GUITextInstance> GetText() { return data.text.lock(); }

	GUIScriptVar Get(const std::string& name);
	std::shared_ptr<GUISpriteInstance> GetSprite(const std::string& name);
	std::shared_ptr<GUITextInstance> GetText(const std::string& name);
	void Set(const std::string& name, const GUIScriptVar& value);

	std::shared_ptr<GUISpriteInstance> GetNestedSprite() { return data.sprite.lock(); }
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

	struct guiObjectData_t {
		std::weak_ptr<GUISpriteInstance> sprite;		// only valid if objectType == SWF_OBJECT_SPRITE
		std::weak_ptr<GUITextInstance> text;			// only valid if objectType == SWF_OBJECT_TEXT
	} data;

	guiNamedVar_t* GetVariable(const std::string& name, bool create);
};
/*
inline bool operator==(const GUIScriptObject::guiNamedVar_t& l, const GUIScriptObject::guiNamedVar_t& r)
{
	return l.name == r.name && l.value == r.value;
}*/

#endif
