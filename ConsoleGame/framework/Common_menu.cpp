#include "Common_local.h"
#include "../renderer/tr_local.h"
#include "../d3xp/Game_local.h"

/*
==============
idCommonLocal::StartMainMenu
==============
*/
void idCommonLocal::StartMenu(bool playIntro) {
	if (game && game->Shell_IsActive()) {
		return;
	}

	if (game) {
		game->Shell_Show(true);
	}
}

/*
===============
idCommonLocal::ExitMenu
===============
*/
void idCommonLocal::ExitMenu() {
	if (game) {
		game->Shell_Show(false);
	}
}

/*
==============
idCommonLocal::MenuEvent

Executes any commands returned by the gui
==============
*/
bool idCommonLocal::MenuEvent(const sysEvent_t* event) {
	if (game && game->Shell_IsActive()) {
		return game->Shell_HandleGuiEvent(event);
	}

	return false;
}