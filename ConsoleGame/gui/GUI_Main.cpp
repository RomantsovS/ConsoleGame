#include "GUI.h"

/*
===================
GUI::GUI
===================
*/
GUI::GUI(const std::string& filename_) :
	filename(filename_),
	isActive(false) {

	globals = std::make_shared<GUIScriptObject>();

	mainspriteInstance = std::make_shared<GUISpriteInstance>();
	mainspriteInstance->Init();

	shortcutKeys = std::make_shared<GUIScriptObject>();
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
