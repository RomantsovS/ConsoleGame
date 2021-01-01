#include "MenuHandler.h"

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::idMenuHandler() :
	activeScreen(-1),
	nextScreen(-1),
	isGUIActive(false) {
	menuScreens.resize(MAX_SCREEN_AREAS);
}

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::~idMenuHandler() {
	Cleanup();
}

/*
================================================
idMenuHandler::Initialize
================================================
*/
void idMenuHandler::Initialize() {
	Cleanup();
}

/*
================================================
idMenuHandler::Cleanup
================================================
*/
void idMenuHandler::Cleanup() {
	for (int index = 0; index < MAX_SCREEN_AREAS; ++index) {
		if (menuScreens[index]) {
			menuScreens[index] = nullptr;
		}
	}
}

/*
================================================
idMenuHandler::IsActive
================================================
*/
bool idMenuHandler::IsActive() {
	return isGUIActive;
}

/*
================================================
idMenuHandler::ActivateMenu
================================================
*/
void idMenuHandler::ActivateMenu(bool show) {

	if (!show) {
		isGUIActive = show;
		return;
	}

	isGUIActive = show;
}

/*
================================================
idMenuHandler::Update
================================================
*/
void idMenuHandler::Update() {
	if (isGUIActive) {
		
	}
}
