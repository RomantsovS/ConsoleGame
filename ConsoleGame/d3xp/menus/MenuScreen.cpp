#include "MenuScreen.h"

idMenuScreen::idMenuScreen() {
}

idMenuScreen::~idMenuScreen() {
}

/*
========================
idMenuScreen::Update
========================
*/
void idMenuScreen::Update() {
	if (!menuGUI) {
		return;
	}

	//
	// Display
	//
	for (size_t childIndex = 0; childIndex < GetChildren().size(); ++childIndex) {
		GetChildByIndex(childIndex)->Update();
	}

	if (menuData) {
		menuData->UpdateChildren();
	}
}

/*
========================
idMenuScreen::HideScreen
========================
*/
void idMenuScreen::HideScreen() {
	if (!menuGUI) {
		return;
	}

	if (!BindSprite(menuGUI->GetRootObject())) {
		return;
	}

	Update();
}

/*
========================
idMenuScreen::ShowScreen
========================
*/
void idMenuScreen::ShowScreen() {
	if (!menuGUI) {
		return;
	}

	if (!BindSprite(menuGUI->GetRootObject())) {
		return;
	}

	GetSprite()->SetVisible(true);

	Update();
}
