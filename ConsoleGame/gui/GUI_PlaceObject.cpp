#include "GUI_SpriteInstance.h"

/*
========================
idSWFSpriteInstance::PlaceObject3
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
	display = AddDisplayEntry(depth);
	if (display == nullptr) {
		common->Warning("PlaceObject: trying to create a new entry at %d, but an item already exists there", depth);
		return;
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

