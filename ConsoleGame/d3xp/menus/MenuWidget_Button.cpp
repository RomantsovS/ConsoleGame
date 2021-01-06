#include "MenuWidget.h"
/*
========================
idMenuWidget_Button::Update
========================
*/
void idMenuWidget_Button::Update() {
	if (menuData != NULL && menuData->GetGUI()) {
		BindSprite(menuData->GetGUI()->GetRootObject());
	}

	if (!GetSprite()) {
		return;
	}
}
