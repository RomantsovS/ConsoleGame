#include "GUI.h"

/*
===================
GUI::GUI
===================
*/
GUI::GUI(const std::string& filename_) :
	filename(filename_),
	isActive(false) {
}

/*
===================
GUI::Init
===================
*/
void GUI::Init() {
	globals = std::make_shared<GUIScriptObject>();

	mainspriteInstance = std::make_shared<GUISpriteInstance>(shared_from_this());
	mainspriteInstance->Init();
	mainspriteInstance->name = "mainspriteInstance";

	shortcutKeys = std::make_shared<GUIScriptObject>();
	shortcutKeys->Set("K_ENTER", "ENTER");
	globals->Set("shortcutKeys", shortcutKeys);

	mainspriteInstance->RunTo(0);
}

/*
===================
GUI::Activate
===================
*/
void GUI::Activate(bool b) {
	isActive = b;
}
