#ifndef GUI_GUI_SCRIPTVAR_H_
#define GUI_GUI_SCRIPTVAR_H_

#include <string>
#include <memory>

class GUIScriptObject;
class GUISpriteInstance;

/*
========================
A variable in an action script
these can be on the stack, in a script object, passed around as parameters, etc
they can contain raw data (int, float), strings, functions, or objects
========================
*/
class GUIScriptVar {
public:
	GUIScriptVar() { }
	GUIScriptVar(const GUIScriptVar& other);
	GUIScriptVar(std::shared_ptr<GUIScriptObject> o) { SetObject(o); }
	GUIScriptVar(int i) { SetInteger(i); }
	~GUIScriptVar();

	GUIScriptVar& operator=(const GUIScriptVar& other);

	void SetInteger(int i) { Free(); /*value.i = i;*/ }

	void SetObject(std::shared_ptr<GUIScriptObject> o);

	std::shared_ptr<GUIScriptObject> GetObjectScript() { return value.object; }
	std::shared_ptr<GUIScriptObject> GetObjectScript() const { return value.object; }
	std::shared_ptr<GUISpriteInstance> ToSprite();

	bool IsObject() const { return (true); }
private:
	void Free();

	struct guiScriptVarValue_t {
		guiScriptVarValue_t() {}
		~guiScriptVarValue_t() {}

		guiScriptVarValue_t& operator=(const guiScriptVarValue_t& other) {
			if (this != &other) {
				object = other.object;
			}return *this;
		}

		std::shared_ptr<GUIScriptObject> object;
	} value;
};
/*
inline bool operator==(const GUIScriptVar& l, const GUIScriptVar& r) {
	return l.value == r.value;
}

bool operator==(const GUIScriptVar::guiScriptVarValue_t& l, const GUIScriptVar::guiScriptVarValue_t& r) {
	return l.object == r.object;
}
*/
#endif
