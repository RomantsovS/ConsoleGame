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
	//
	// Display
	//
	/*for (int childIndex = 0; childIndex < GetChildren().Num(); ++childIndex) {
		GetChildByIndex(childIndex).Update();
	}

	if (menuData != NULL) {
		menuData->UpdateChildren();
	}*/
}

/*
========================
idMenuScreen::HideScreen
========================
*/
void idMenuScreen::HideScreen() {
	Update();
}

/*
========================
idMenuScreen::ShowScreen
========================
*/
void idMenuScreen::ShowScreen() {
	Update();
}

