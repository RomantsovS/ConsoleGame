#ifndef GUI_GUI_SCRIPTOBJECT_H_
#define GUI_GUI_SCRIPTOBJECT_H_

class idSWFSpriteInstance;

/*
========================
An object in an action script is a collection of variables. functions are also variables.
========================
*/
class idSWFScriptObject : public std::enable_shared_from_this<idSWFScriptObject> {
public:
	idSWFScriptObject();
	~idSWFScriptObject();

	void Clear();

	void SetSprite(std::shared_ptr<idSWFSpriteInstance> s) { objectType = swfObjectType_t::SWF_OBJECT_SPRITE;  data.sprite = s; }
	std::shared_ptr<idSWFSpriteInstance> GetSprite() { return (objectType == swfObjectType_t::SWF_OBJECT_SPRITE) ? data.sprite.lock() : nullptr; }

	void SetText(std::shared_ptr<idSWFTextInstance> t) { objectType = swfObjectType_t::SWF_OBJECT_TEXT; data.text = t; }
	std::shared_ptr<idSWFTextInstance> GetText() { return data.text.lock(); }

	idSWFScriptVar Get(const std::string& name);
	std::shared_ptr<idSWFSpriteInstance> GetSprite(const std::string& name);
	std::shared_ptr<idSWFTextInstance> GetText(const std::string& name);
	void Set(const std::string& name, const idSWFScriptVar& value);
	std::shared_ptr<idSWFScriptObject> GetObjectScript(const std::string& name);

	idSWFScriptVar GetNestedVar(const char* arg1, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL, const char* arg6 = NULL);
	std::shared_ptr<idSWFScriptObject> GetNestedObj(const char* arg1, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL, const char* arg6 = NULL);
	std::shared_ptr<idSWFSpriteInstance> GetNestedSprite(const char* arg1, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL, const char* arg6 = NULL);
	std::shared_ptr<idSWFTextInstance> GetNestedText(const char* arg1, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL, const char* arg6 = NULL);
private:
	struct guiNamedVar_t {
		guiNamedVar_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
			common->DPrintf("%s ctor\n", "guiNamedVar_t");
#endif // DEBUG_PRINT_Ctor_Dtor
		}
		~guiNamedVar_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
			if(isCommonExists)
				common->DPrintf("%s dtor\n", "guiNamedVar_t");
#endif // DEBUG_PRINT_Ctor_Dtor
		}
		//guiNamedVar_t& operator=(const guiNamedVar_t& other);

		int index{ -1 };
		std::string name;
		idSWFScriptVar value;
	};
	std::vector<guiNamedVar_t/*, MyAlloc<guiNamedVar_t>*/> variables;

	enum class swfObjectType_t {
		SWF_OBJECT_OBJECT,
		SWF_OBJECT_ARRAY,
		SWF_OBJECT_SPRITE,
		SWF_OBJECT_TEXT
	} objectType;

	struct guiObjectData_t {
		std::weak_ptr<idSWFSpriteInstance> sprite;		// only valid if objectType == SWF_OBJECT_SPRITE
		std::weak_ptr<idSWFTextInstance> text;			// only valid if objectType == SWF_OBJECT_TEXT
	} data;

	guiNamedVar_t* GetVariable(const std::string& name, bool create);
};
#endif
