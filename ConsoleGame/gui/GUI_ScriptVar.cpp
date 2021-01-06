#include "GUI.h"

/*
========================
GUIScriptVar::GUIScriptVar
========================
*/
GUIScriptVar::GUIScriptVar(const GUIScriptVar& other) {
	type = other.type;
	value = other.value;
}

/*
========================
idSWFScriptVar::operator=
========================
*/
GUIScriptVar& GUIScriptVar::operator=(const GUIScriptVar& other) {
	if (this != &other) {
		Free();
		type = other.type;
		value = other.value;
	}
	return *this;
}

/*
========================
GUIScriptVar::~GUIScriptVar
========================
*/
GUIScriptVar::~GUIScriptVar() {
	Free();
}

/*
========================
idSWFScriptVar::Free
========================
*/
void GUIScriptVar::Free() {
	value.object = nullptr;
}

/*
========================
idSWFScriptVar::SetObject
========================
*/
void GUIScriptVar::SetObject(std::shared_ptr<GUIScriptObject> o) {
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
GUIScriptVar::ToString
========================
*/
std::string GUIScriptVar::ToString() const {
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
	default: common->Error("default case in GUIScriptVar::ToString()"); return "";
	}
}

/*
========================
GUIScriptVar::ToBool
========================
*/
bool GUIScriptVar::ToBool() const {
	switch (type) {
	case swfScriptVarType::SWF_VAR_STRING:	return (value.string->compare("true") == 0 || value.string->compare("1") == 0);

	//case swfScriptVarType::SWF_VAR_FLOAT:		return (value.f != 0.0f);
	//case swfScriptVarType::SWF_VAR_BOOL:		return value.b;
	case swfScriptVarType::SWF_VAR_INTEGER:	return value.i != 0;

	//case swfScriptVarType::SWF_VAR_OBJECT:	return value.object->DefaultValue(false).ToBool();

	case swfScriptVarType::SWF_VAR_FUNCTION:
	case swfScriptVarType::SWF_VAR_NULL:
	case swfScriptVarType::SWF_VAR_UNDEF:		return false;
	default: common->Error("default case in GUIScriptVar::ToBool()"); return false;
	}
}

/*
========================
GUIScriptVar::ToSprite
========================
*/
std::shared_ptr<GUISpriteInstance> GUIScriptVar::ToSprite() {
	if (IsObject() && value.object) {
		return value.object->GetSprite();
	}

	return nullptr;
}

