#include "GUI.h"

/*
========================
GUISpriteInstance::GUISpriteInstance
========================
*/
GUISpriteInstance::GUISpriteInstance(std::shared_ptr<GUI> _gui) :
	gui(_gui),
	isVisible(true),
	scriptObject(nullptr),
	isSprite(false)
{
}

/*
========================
idSWFSpriteInstance::Init
========================
*/
void GUISpriteInstance::Init() {
	scriptObject = std::make_shared<GUIScriptObject>();
	scriptObject->SetSprite(shared_from_this());
}

/*
========================
GUISpriteInstance::~GUISpriteInstance
========================
*/
GUISpriteInstance::~GUISpriteInstance() {
	//FreeDisplayList();
	displayList.clear();
}

/*
========================
GUISpriteInstance::FindDisplayEntry
========================
*/
guiDisplayEntry_t* GUISpriteInstance::FindDisplayEntry(int depth) {
	int len = displayList.size();
	int mid = len;
	int offset = 0;
	while (mid > 0) {
		mid = len >> 1;
		if (displayList[offset + mid].depth <= depth) {
			offset += mid;
		}
		len -= mid;
	}
	if (displayList[offset].depth == depth) {
		return &displayList[offset];
	}
	return nullptr;
}

/*
========================
GUISpriteInstance::AddDisplayEntry
========================
*/
guiDisplayEntry_t* GUISpriteInstance::AddDisplayEntry(int depth, bool createText) {
	size_t i = 0;
	for (; i < displayList.size(); i++) {
		if (displayList[i].depth == depth) {
			return NULL;
		}
		if (displayList[i].depth > depth) {
			break;
		}
	}

	displayList.emplace_back();
	guiDisplayEntry_t& display = displayList.back();
	display.depth = depth;

	if (!createText) {
		display.spriteInstance = std::make_shared<GUISpriteInstance>(GetGUI());
		display.spriteInstance->Init();
		display.spriteInstance->isSprite = true;
		display.spriteInstance->RunTo(1);
	}
	else {
		display.textInstance = std::make_shared<GUITextInstance>();
		display.textInstance->Init(GetGUI());
	}

	return &display;
}

/*
===================
GUISpriteInstance::RunTo
===================
*/
void GUISpriteInstance::RunTo(int targetFrame) {
	PlaceObject();
}

/*
========================
idSWFSpriteInstance::SetVisible
========================
*/
void GUISpriteInstance::SetVisible(bool visible) {
	isVisible = visible;
}

/*
========================
idSWFSpriteInstance::SetXPos
========================
*/
void GUISpriteInstance::SetXPos(float xPos) {
	/*if (parent == NULL) {
		return;
	}

	swfDisplayEntry_t* thisDisplayEntry = parent->FindDisplayEntry(depth);
	if (thisDisplayEntry == NULL || thisDisplayEntry->spriteInstance != this) {
		idLib::Warning("_y: Couldn't find our display entry in our parents display list");
		return;
	}

	thisDisplayEntry->matrix.tx = xPos;*/
}

