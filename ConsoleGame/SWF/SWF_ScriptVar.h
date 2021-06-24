#ifndef GUI_GUI_SCRIPTVAR_H_
#define GUI_GUI_SCRIPTVAR_H_

class idSWFScriptObject;
class idSWFScriptFunction;

/*
========================
A variable in an action script
these can be on the stack, in a script object, passed around as parameters, etc
they can contain raw data (int, float), strings, functions, or objects
========================
*/
class idSWFScriptVar {
public:
	idSWFScriptVar() : type(swfScriptVarType::SWF_VAR_UNDEF) { }
	idSWFScriptVar(const idSWFScriptVar& other);
	idSWFScriptVar(std::shared_ptr<idSWFScriptObject> o) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetObject(o); }
	idSWFScriptVar(const std::string& s) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetString(s); }
	idSWFScriptVar(const char* s) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetString(std::string(s)); }
	idSWFScriptVar(int i) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetInteger(i); }
	idSWFScriptVar(std::shared_ptr<idSWFScriptFunction> nf) : type(swfScriptVarType::SWF_VAR_UNDEF) { SetFunction(nf); }
	~idSWFScriptVar();

	idSWFScriptVar& operator=(const idSWFScriptVar& other);

	void SetString(const std::string& s) {
		Free();
		type = swfScriptVarType::SWF_VAR_STRING;
#ifdef DEBUG
		value.string = std::shared_ptr<std::string>(DBG_NEW std::string(s));
#else
		value.string = std::make_shared<std::string>(s);
#endif
	}
	void SetFloat(float f) { Free(); type = swfScriptVarType::SWF_VAR_FLOAT; value.f = f; }
	void SetBool(bool b) { Free(); type = swfScriptVarType::SWF_VAR_BOOL; value.b = b; }
	void SetInteger(int i) { Free(); type = swfScriptVarType::SWF_VAR_INTEGER; value.i = i; }

	void SetObject(std::shared_ptr<idSWFScriptObject> o);
	void SetFunction(std::shared_ptr<idSWFScriptFunction> f);

	std::string ToString() const;
	bool ToBool() const;
	int ToInteger() const;

	std::shared_ptr<idSWFScriptObject> GetObjectScript() { return value.object.lock(); }
	std::shared_ptr<idSWFScriptObject> GetObjectScript() const { return value.object.lock(); }
	std::shared_ptr<idSWFScriptFunction> GetFunction() { return value.function; }
	std::shared_ptr<idSWFSpriteInstance> ToSprite();
	std::shared_ptr<idSWFTextInstance> ToText();

	bool IsString() const { return (type == swfScriptVarType::SWF_VAR_STRING) || (type == swfScriptVarType::SWF_VAR_STRINGID); }
	bool IsFunction() const { return (type == swfScriptVarType::SWF_VAR_FUNCTION); }
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

	struct idSWFScriptVarValue_t {
		idSWFScriptVarValue_t() {}
		~idSWFScriptVarValue_t() {}

		idSWFScriptVarValue_t& operator=(const idSWFScriptVarValue_t& other) {
			if (this != &other) {
				f = other.f;
				i = other.i;
				b = other.b;
				object = other.object;
				string = other.string;
				function = other.function;
			}return *this;
		}

		float f;
		int i;
		bool b;
		std::weak_ptr<idSWFScriptObject> object;
		std::shared_ptr<std::string> string;
		std::shared_ptr<idSWFScriptFunction> function;
	} value;
};
#endif
