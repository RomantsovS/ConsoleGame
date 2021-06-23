#pragma hdrstop
#include "../idlib/precompiled.h"

/*
========================
idSWFScriptVar::idSWFScriptVar
========================
*/
idSWFScriptVar::idSWFScriptVar(const idSWFScriptVar& other) {
	type = other.type;
	value = other.value;
}

/*
========================
idSWFScriptVar::operator=
========================
*/
idSWFScriptVar& idSWFScriptVar::operator=(const idSWFScriptVar& other) {
	if (this != &other) {
		Free();
		type = other.type;
		value = other.value;
	}
	return *this;
}

/*
========================
idSWFScriptVar::~idSWFScriptVar
========================
*/
idSWFScriptVar::~idSWFScriptVar() {
	Free();
}

/*
========================
idSWFScriptVar::Free
========================
*/
void idSWFScriptVar::Free() {
	value.string = nullptr;
	value.function.reset();
	value.object.reset();
	type = swfScriptVarType::SWF_VAR_UNDEF;
}

/*
========================
idSWFScriptVar::SetObject
========================
*/
void idSWFScriptVar::SetObject(std::shared_ptr<idSWFScriptObject> o) {
	Free();
	if (!o) {
		type = swfScriptVarType::SWF_VAR_NULL;
	}
	else {
		type = swfScriptVarType::SWF_VAR_OBJECT;
		value.object = o;
	}
}

/*
========================
idSWFScriptVar::SetFunction
========================
*/
void idSWFScriptVar::SetFunction(std::shared_ptr<idSWFScriptFunction> f) {
	Free();
	if (!f) {
		type = swfScriptVarType::SWF_VAR_NULL;
	}
	else {
		type = swfScriptVarType::SWF_VAR_FUNCTION;
		value.function = f;
	}
}

/*
========================
idSWFScriptVar::ToString
========================
*/
std::string idSWFScriptVar::ToString() const {
	switch (type) {
	//case swfScriptVarType::SWF_VAR_STRINGID:	return std::string(value.i).GetLocalizedString();
	case swfScriptVarType::SWF_VAR_STRING:	return *value.string;

	//case swfScriptVarType::SWF_VAR_FLOAT:		return va("%g", value.f);
	//case swfScriptVarType::SWF_VAR_BOOL:		return value.b ? "true" : "false";
	//case swfScriptVarType::SWF_VAR_INTEGER:	return va("%i", value.i);

	case swfScriptVarType::SWF_VAR_NULL:		return "[null]";
	case swfScriptVarType::SWF_VAR_UNDEF:		return "[undefined]";
	//case swfScriptVarType::SWF_VAR_OBJECT:	return value.object->DefaultValue(true).ToString();
	case swfScriptVarType::SWF_VAR_FUNCTION:
		/*if (swf_debugShowAddress.GetBool()) {
			return va("[function:%p]", value.function);
		}
		else {*/
			return "[function]";
		//}
	default: common->Error("default case in idSWFScriptVar::ToString()"); return "";
	}
}

/*
========================
idSWFScriptVar::ToBool
========================
*/
bool idSWFScriptVar::ToBool() const {
	switch (type) {
	case swfScriptVarType::SWF_VAR_STRING:	return (value.string->compare("true") == 0 || value.string->compare("1") == 0);

	//case swfScriptVarType::SWF_VAR_FLOAT:		return (value.f != 0.0f);
	case swfScriptVarType::SWF_VAR_BOOL:		return value.b;
	case swfScriptVarType::SWF_VAR_INTEGER:	return value.i != 0;

	//case swfScriptVarType::SWF_VAR_OBJECT:	return value.object->DefaultValue(false).ToBool();

	case swfScriptVarType::SWF_VAR_FUNCTION:
	case swfScriptVarType::SWF_VAR_NULL:
	case swfScriptVarType::SWF_VAR_UNDEF:		return false;
	default: common->Error("default case in idSWFScriptVar::ToBool()"); return false;
	}
}

/*
========================
idSWFScriptVar::ToInteger
========================
*/
int idSWFScriptVar::ToInteger() const {
	switch (type) {
	case swfScriptVarType::SWF_VAR_STRING:	return atoi(value.string->c_str());

	//case swfScriptVarType::SWF_VAR_FLOAT:		return idMath::Ftoi(value.f);

	case swfScriptVarType::SWF_VAR_BOOL:		return value.b ? 1 : 0;
	case swfScriptVarType::SWF_VAR_INTEGER:	return value.i;

	//case swfScriptVarType::SWF_VAR_OBJECT:	return value.object->DefaultValue(false).ToInteger();

	case swfScriptVarType::SWF_VAR_FUNCTION:
	case swfScriptVarType::SWF_VAR_NULL:
	case swfScriptVarType::SWF_VAR_UNDEF:		return 0;
	default: common->Error("default case in idSWFScriptVar::ToInteger()"); return 0;
	}
}

/*
========================
idSWFScriptVar::ToSprite
========================
*/
std::shared_ptr<idSWFSpriteInstance> idSWFScriptVar::ToSprite() {
	if (IsObject() && !value.object.expired()) {
		return value.object.lock()->GetSprite();
	}

	return nullptr;
}

/*
========================
idSWFScriptVar::ToText
========================
*/
std::shared_ptr<idSWFTextInstance> idSWFScriptVar::ToText() {
	if (IsObject() && !value.object.expired()) {
		return value.object.lock()->GetText();
	}

	return nullptr;
}
