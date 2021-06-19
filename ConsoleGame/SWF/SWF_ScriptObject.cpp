#pragma hdrstop
#include "../idlib/precompiled.h"

/*
========================
idSWFScriptObject::guiNamedVar_t::operator=
========================
*//*
idSWFScriptObject::guiNamedVar_t& idSWFScriptObject::guiNamedVar_t::operator=(const guiNamedVar_t& other) {
	if (&other != this) {
		index = other.index;
		name = other.name;
		value = other.value;
	}
	return *this;
}*/

/*
========================
idSWFScriptObject::idSWFScriptObject
========================
*/
idSWFScriptObject::idSWFScriptObject() : objectType(swfObjectType_t::SWF_OBJECT_OBJECT) {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idSWFScriptObject");
#endif // DEBUG_PRINT_Ctor_Dtor
	data.sprite.reset();
	data.text.reset();
}

/*
========================
idSWFScriptObject::~idSWFScriptObject
========================
*/
idSWFScriptObject::~idSWFScriptObject() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idSWFScriptObject");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
========================
idSWFScriptObject::Clear
========================
*/
void idSWFScriptObject::Clear() {
	variables.clear();
}

/*
========================
idSWFScriptObject::Get
========================
*/
idSWFScriptVar idSWFScriptObject::Get(const std::string& name) {
	guiNamedVar_t* variable = GetVariable(name, false);
	if (variable == nullptr) {
		return idSWFScriptVar();
	}
	else {
		return variable->value;
	}
}

/*
========================
idSWFScriptObject::GetSprite
========================
*/
std::shared_ptr<idSWFSpriteInstance> idSWFScriptObject::GetSprite(const std::string& name) {
	idSWFScriptVar var = Get(name);
	return var.ToSprite();
}

/*
========================
idSWFScriptObject::GetObject
========================
*/
std::shared_ptr<idSWFScriptObject> idSWFScriptObject::GetObjectScript(const std::string& name) {
	idSWFScriptVar var = Get(name);
	if (var.IsObject()) {
		return var.GetObjectScript();
	}
	return nullptr;
}

/*
========================
idSWFScriptObject::GetText
========================
*/
std::shared_ptr<idSWFTextInstance> idSWFScriptObject::GetText(const std::string& name) {
	idSWFScriptVar var = Get(name);
	if (var.IsObject()) {
		return var.GetObjectScript()->GetText();
	}
	return nullptr;
}

/*
========================
idSWFScriptObject::Set
========================
*/
void idSWFScriptObject::Set(const std::string& name, const idSWFScriptVar& value) {
	guiNamedVar_t* variable = GetVariable(name, true);

	variable->value = value;
}

/*
========================
idSWFScriptObject::GetVariable
========================
*/
idSWFScriptObject::guiNamedVar_t* idSWFScriptObject::GetVariable(const std::string& name, bool create) {
	for (auto& var : variables) {
		if (var.name == name)
			return &var;
	}

	if (create) {
		variables.emplace_back();
		guiNamedVar_t* variable = &variables.back();
		variable->index = atoi(name.c_str());
		if (variable->index == 0 && name == "0") {
			variable->index = -1;
		}
		variable->name = name;
		return variable;
	}
	return nullptr;
}

/*
========================
idSWFScriptObject::GetNestedVar
========================
*/
idSWFScriptVar idSWFScriptObject::GetNestedVar(const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6) {
	const char* const args[] = { arg1, arg2, arg3, arg4, arg5, arg6 };
	const int numArgs = sizeof(args) / sizeof(const char*);

	std::vector<std::string> vars;
	vars.reserve(numArgs);
	for (const auto& str : args) {
		if (str)
			vars.push_back(str);
		else
			break;
	}

	std::shared_ptr<idSWFScriptObject> baseObject = shared_from_this();
	idSWFScriptVar retVal;

	for (size_t i = 0; i < vars.size(); ++i) {
		idSWFScriptVar var = baseObject->Get(vars[i]);

		// when at the end of object path just use the latest value as result
		if (i == vars.size() - 1) {
			retVal = var;
			break;
		}

		// encountered variable in path that wasn't an object
		if (!var.IsObject()) {
			retVal = idSWFScriptVar();
			break;
		}

		baseObject = var.GetObjectScript();
	}

	return retVal;
}

/*
========================
idSWFScriptObject::GetNestedObj
========================
*/
std::shared_ptr<idSWFScriptObject> idSWFScriptObject::GetNestedObj(const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6) {
	idSWFScriptVar var = GetNestedVar(arg1, arg2, arg3, arg4, arg5, arg6);

	if (!var.IsObject()) {
		return nullptr;
	}

	return var.GetObjectScript();
}

/*
========================
idSWFScriptObject::GetNestedSprite
========================
*/
std::shared_ptr<idSWFSpriteInstance> idSWFScriptObject::GetNestedSprite(const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6) {
	idSWFScriptVar var = GetNestedVar(arg1, arg2, arg3, arg4, arg5, arg6);
	return var.ToSprite();

}

/*
========================
idSWFScriptObject::GetNestedText
========================
*/
std::shared_ptr<idSWFTextInstance> idSWFScriptObject::GetNestedText(const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5, const char* arg6) {
	idSWFScriptVar var = GetNestedVar(arg1, arg2, arg3, arg4, arg5, arg6);
	return var.ToText();

}
