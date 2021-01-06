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
	GUIScriptVar() : type(swfScriptVarType::SWF_VAR_UNDEF) { }
	GUIScriptVar(const GUIScriptVar& other);
	GUIScriptVar(std::shared_ptr<GUIScriptObject> o) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetObject(o); }
	GUIScriptVar(const std::string& s) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetString(s); }
	GUIScriptVar(const char* s) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetString(std::string(s)); }
	GUIScriptVar(int i) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetInteger(i); }
	~GUIScriptVar();

	GUIScriptVar& operator=(const GUIScriptVar& other);

	void SetString(const std::string& s) { Free(); type = swfScriptVarType::SWF_VAR_STRINGID; value.string = std::make_shared<std::string>(s); }
	void SetInteger(int i) { Free(); type = swfScriptVarType::SWF_VAR_INTEGER; value.i = i; }

	void SetObject(std::shared_ptr<GUIScriptObject> o);

	std::string ToString() const;
	bool ToBool() const;

	std::shared_ptr<GUIScriptObject> GetObjectScript() { return value.object; }
	std::shared_ptr<GUIScriptObject> GetObjectScript() const { return value.object; }
	std::shared_ptr<GUISpriteInstance> ToSprite();

	bool IsString() const { return (type == swfScriptVarType::SWF_VAR_STRING) || (type == swfScriptVarType::SWF_VAR_STRINGID); }
	bool IsObject() const { return (type == swfScriptVarType::SWF_VAR_OBJECT); }

	enum class swfScriptVarType {
		SWF_VAR_STRINGID,
		SWF_VAR_STRING,
		SWF_VAR_FLOAT,
		SWF_VAR_NULL,
		SWF_VAR_UNDEF,
		SWF_VAR_BOOL,
		SWF_VAR_INTEGER,
		SWF_VAR_FUNCTION,
		SWF_VAR_OBJECT
	};

	swfScriptVarType GetType() const { return type; }

private:
	void Free();
	swfScriptVarType type;

	struct guiScriptVarValue_t {
		guiScriptVarValue_t() {}
		~guiScriptVarValue_t() {}

		guiScriptVarValue_t& operator=(const guiScriptVarValue_t& other) {
			if (this != &other) {
				i = other.i;
				object = other.object;
				string = other.string;
			}return *this;
		}

		int i;
		std::shared_ptr<GUIScriptObject> object;
		std::shared_ptr<std::string> string;
	} value;
};
#endif
