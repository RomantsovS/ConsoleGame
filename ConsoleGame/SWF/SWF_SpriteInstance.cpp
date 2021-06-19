#pragma hdrstop
#include "../idlib/precompiled.h"

/*
========================
idSWFSpriteInstance::idSWFSpriteInstance
========================
*/
idSWFSpriteInstance::idSWFSpriteInstance(std::shared_ptr<idSWF> _swf) :
	swf(_swf),
	isVisible(false),
	scriptObject(nullptr) {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idSWFSpriteInstance");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
========================
idSWFSpriteInstance::Init
========================
*/
void idSWFSpriteInstance::Init() {
#ifdef DEBUG
	scriptObject = std::shared_ptr<idSWFScriptObject>(DBG_NEW idSWFScriptObject);
#else
	scriptObject = std::make_shared<idSWFScriptObject>();
#endif

	scriptObject->SetSprite(shared_from_this());
}

/*
========================
idSWFSpriteInstance::~idSWFSpriteInstance
========================
*/
idSWFSpriteInstance::~idSWFSpriteInstance() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idSWFSpriteInstance");
#endif // DEBUG_PRINT_Ctor_Dtor
}

/*
========================
idSWFSpriteInstance::FreeDisplayList
========================
*/
void idSWFSpriteInstance::FreeDisplayList() {
	for (size_t i = 0; i < displayList.size(); i++) {
		if (displayList[i].spriteInstance)
			displayList[i].spriteInstance->Clear();
		if (displayList[i].textInstance)
			displayList[i].textInstance->Clear();
	}
	displayList.clear();	// not calling Clear() so we don't continuously re-allocate memory
}

void idSWFSpriteInstance::Clear() {
	FreeDisplayList();
	displayList.clear();
	scriptObject->SetSprite(nullptr);
	scriptObject->Clear();
	scriptObject = nullptr;
}

/*
========================
idSWFSpriteInstance::FindDisplayEntry
========================
*/
swfDisplayEntry_t* idSWFSpriteInstance::FindDisplayEntry(int depth) {
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
idSWFSpriteInstance::AddDisplayEntry
========================
*/
swfDisplayEntry_t* idSWFSpriteInstance::AddDisplayEntry(int depth, bool isSprite) {
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
	swfDisplayEntry_t& display = displayList.back();
	display.depth = depth;

	if (isSprite) {
#ifdef DEBUG
		display.spriteInstance = std::shared_ptr<idSWFSpriteInstance>(DBG_NEW idSWFSpriteInstance(GetSWF()));
#else
		display.spriteInstance = std::make_shared<idSWFSpriteInstance>(GetSWF());
#endif
		display.spriteInstance->Init();
	}
	else {
#ifdef DEBUG
		display.textInstance = std::shared_ptr<idSWFTextInstance>(DBG_NEW idSWFTextInstance());
#else
		display.textInstance = std::make_shared<idSWFTextInstance>();
#endif
		display.textInstance->Init(GetSWF());
	}

	return &display;
}

/*
===================
idSWFSpriteInstance::RunTo
===================
*/
void idSWFSpriteInstance::RunTo(int targetFrame) {
	FreeDisplayList();
}

/*
========================
idSWFSpriteInstance::SetVisible
========================
*/
void idSWFSpriteInstance::SetVisible(bool visible) {
	isVisible = visible;
}

/*
========================
idSWFSpriteInstance::SetColor
========================
*/
void idSWFSpriteInstance::SetColor(const Screen::ConsoleColor color) {
	for (auto& display : displayList) {
		if (display.textInstance) {
			display.textInstance->color = color;
		}
	}
}

/*
========================
idSWFSpriteInstance::SetXPos
========================
*/
void idSWFSpriteInstance::SetXPos(float xPos) {
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

