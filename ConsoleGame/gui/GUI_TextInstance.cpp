#include "GUI.h"

/*
========================
GUITextInstance::Init
========================
*/
void GUITextInstance::Init(std::shared_ptr<GUI> _gui) {
	gui = _gui;

	text = "initial text";
	color = Screen::ConsoleColor::White;
	visible = true;

	scriptObject->SetText(shared_from_this());
}
