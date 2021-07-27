#include <precompiled.h>
#pragma hdrstop

/*
========================
idSWFSpriteInstance::PlaceObject
========================
*/
swfDisplayEntry_t* idSWFSpriteInstance::PlaceObject(size_t depth, bool isSprite, const std::string& name, const Vector2& pos) {
	swfDisplayEntry_t* display;
	/*display = FindDisplayEntry(depth);
	if (display != nullptr) {
		return;
	}*/

	// create a new entry
	display = AddDisplayEntry(depth, isSprite);
	if (display == nullptr) {
		common->Error("PlaceObject: trying to create a new entry \"%s\" at %d, but an item already exists there", name.c_str(), depth);
		return nullptr;
	}

	display->pos = pos;

	if (display->spriteInstance) {
		display->spriteInstance->name = name;
		scriptObject->Set(name, display->spriteInstance->GetScriptObject());
	}
	else if (display->textInstance) {
		scriptObject->Set(name, display->textInstance->GetScriptObject());
	}

	return display;
}

/*
========================
idSWFSpriteInstance::RemoveObject
========================
*/
void idSWFSpriteInstance::RemoveObject() noexcept {
	//RemoveDisplayEntry();
}

