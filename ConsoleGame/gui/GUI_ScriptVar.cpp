#include "GUI_ScriptVar.h"

/*
========================
GUIScriptVar::GUIScriptVar
========================
*/
GUIScriptVar::GUIScriptVar(const GUIScriptVar& other) {
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
		//type = SWF_VAR_NULL;
	}
	else {
		//type = SWF_VAR_OBJECT;
		value.object = o;
	}
}

/*
========================
GUIScriptVar::ToSprite
========================
*/
std::shared_ptr<GUISpriteInstance> GUIScriptVar::ToSprite() {
	/*if (IsObject() && value.object != NULL) {
		return value.object->GetSprite();
	}*/

	return nullptr;
}

