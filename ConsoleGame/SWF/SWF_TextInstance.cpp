#pragma hdrstop
#include "../idlib/precompiled.h"

/*
========================
idSWFTextInstance::Init
========================
*/
void idSWFTextInstance::Init(std::shared_ptr<idSWF> _swf) {
	swf = _swf;

	text = "initial text";
	color = Screen::ConsoleColor::White;
	visible = true;

	scriptObject->SetText(shared_from_this());
}

/*
========================
idSWFTextInstance::GetTextLength
========================
*/
size_t idSWFTextInstance::GetTextLength() {
	return text.size();
}
