#pragma hdrstop
#include <precompiled.h>

/*
========================
idSWFTextInstance::Init
========================
*/
void idSWFTextInstance::Init(std::shared_ptr<idSWF> _swf) {
	swf = _swf;

	text = "initial text";
	color = colorWhite;
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

void idSWFTextInstance::Clear() {
	scriptObject->SetText(NULL);
	scriptObject->Clear();
	scriptObject = nullptr;
}
