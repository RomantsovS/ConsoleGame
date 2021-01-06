#include "GUI.h"

/*
========================
GUIScriptObject::guiNamedVar_t::operator=
========================
*//*
GUIScriptObject::guiNamedVar_t& GUIScriptObject::guiNamedVar_t::operator=(const guiNamedVar_t& other) {
	if (&other != this) {
		index = other.index;
		name = other.name;
		value = other.value;
	}
	return *this;
}*/

/*
========================
GUIScriptObject::GUIScriptObject
========================
*/
GUIScriptObject::GUIScriptObject() : objectType(swfObjectType_t::SWF_OBJECT_OBJECT) {
	data.sprite.reset();
	data.text.reset();
}

/*
========================
GUIScriptObject::~GUIScriptObject
========================
*/
GUIScriptObject::~GUIScriptObject() {
}

/*
========================
idSWFScriptObject::Get
========================
*/
GUIScriptVar GUIScriptObject::Get(const std::string& name) {
	guiNamedVar_t* variable = GetVariable(name, false);
	if (variable == nullptr) {
		return GUIScriptVar();
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
std::shared_ptr<GUISpriteInstance> GUIScriptObject::GetSprite(const std::string& name) {
	GUIScriptVar var = Get(name);
	return var.ToSprite();
}

/*
========================
GUIScriptObject::GetText
========================
*/
std::shared_ptr<GUITextInstance> GUIScriptObject::GetText(const std::string& name) {
	GUIScriptVar var = Get(name);
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
void GUIScriptObject::Set(const std::string& name, const GUIScriptVar& value) {
	guiNamedVar_t* variable = GetVariable(name, true);

	variable->value = value;
}

/*
========================
idSWFScriptObject::GetVariable
========================
*/
GUIScriptObject::guiNamedVar_t* GUIScriptObject::GetVariable(const std::string& name, bool create) {
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
