#include "GUI.h"

/*
========================
GUISpriteInstance::PlaceObject
========================
*/
void GUISpriteInstance::PlaceObject() {
	size_t depth = 0;

	guiDisplayEntry_t* display;
	/*display = FindDisplayEntry(depth);
	if (display != nullptr) {
		return;
	}*/

	// create a new entry
	display = AddDisplayEntry(depth, isSprite);
	if (display == nullptr) {
		common->Warning("PlaceObject: trying to create a new entry at %d, but an item already exists there", depth);
		return;
	}

	if (display->spriteInstance) {
		std::string name = "joy1";
		display->spriteInstance->name = name;
		scriptObject->Set(name, display->spriteInstance->GetScriptObject());
	}
	else if (display->textInstance) {
		std::string name = "txt_info";
		scriptObject->Set(name, display->textInstance->GetScriptObject());
	}
}

/*
========================
GUISpriteInstance::RemoveObject
========================
*/
void GUISpriteInstance::RemoveObject() {
	//RemoveDisplayEntry();
}

