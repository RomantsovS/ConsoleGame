#include "MenuScreen.h"

void idMenuHandler_Shell::Update() {

	if (nextState != state) {
		if (nextState == shellState_t::SHELL_STATE_PRESS_START) {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_START);
			state = nextState;
		}
		else if (nextState == shellState_t::SHELL_STATE_IDLE) {
			if (nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_START) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_GAME_LOBBY) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_INVALID)) {
				nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_ROOT);
			}
			state = nextState;
		}
		else if (nextState == shellState_t::SHELL_STATE_PARTY_LOBBY) {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY);
			state = nextState;
		}
		else if (nextState == shellState_t::SHELL_STATE_GAME_LOBBY) {
			if (state != shellState_t::SHELL_STATE_IN_GAME) {
				nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_GAME_LOBBY);

				state = nextState;
			}
		}
		else if (nextState == shellState_t::SHELL_STATE_PAUSED) {
			/*if (gameComplete) {
				nextScreen = SHELL_AREA_CREDITS;
			}
			else {*/
				nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_ROOT);
			//}

			state = nextState;
		}
		else if (nextState == shellState_t::SHELL_STATE_CONNECTING) {
			state = nextState;
		}
		else if (nextState == shellState_t::SHELL_STATE_SEARCHING) {
			state = nextState;
		}
	}

	if (activeScreen != nextScreen) {
		//UpdateBGState();

		if (nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_INVALID)) {

			if (activeScreen > static_cast<int>(shellAreas_t::SHELL_AREA_INVALID) && activeScreen < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS) && menuScreens[activeScreen]) {
				menuScreens[activeScreen]->HideScreen();
			}
		}
		else {

			if (activeScreen > static_cast<int>(shellAreas_t::SHELL_AREA_INVALID) && activeScreen < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS) && menuScreens[activeScreen]) {
				menuScreens[activeScreen]->HideScreen();
			}

			if (nextScreen > static_cast<int>(shellAreas_t::SHELL_AREA_INVALID) && nextScreen < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS) && menuScreens[nextScreen]) {
				//menuScreens[nextScreen]->UpdateCmds();
				menuScreens[nextScreen]->ShowScreen();
			}
		}
		activeScreen = nextScreen;
	}

	idMenuHandler::Update();
}

/*
========================
idMenuHandler_Shell::Initialize
========================
*/
void idMenuHandler_Shell::Initialize() {
	idMenuHandler::Initialize();

	//---------------------
	// Initialize the menus
	//---------------------
#define BIND_SHELL_SCREEN( screenId, className, menuHandler )	\
	menuScreens[ (screenId) ] = std::make_shared<className>();	\
	menuScreens[ (screenId) ]->Initialize( menuHandler );
	//menuScreens[ (screenId) ]->AddRef();

	for (int i = 0; i < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS); ++i) {
		menuScreens[i] = nullptr;
	}

	if (inGame) {
	}
	else {
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_NEW_GAME), idMenuScreen_Shell_NewGame, shared_from_this());
	}
}

/*
========================
idMenuHandler_Shell::Cleanup
========================
*/
void idMenuHandler_Shell::Cleanup() {
	idMenuHandler::Cleanup();
}

/*
========================
idMenuHandler_Shell::ActivateMenu
========================
*/
void idMenuHandler_Shell::ActivateMenu(bool show) {

	if (show && IsActive()) {
		return;
	}
	else if (!show && !IsActive()) {
		return;
	}

	idMenuHandler::ActivateMenu(show);
}

