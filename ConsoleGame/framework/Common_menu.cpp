#include <precompiled.h>
#pragma hdrstop

#include "Common_local.h"

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
		game->Shell_SyncWithSession();
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

/*
=================
idCommonLocal::GuiFrameEvents
=================
*/
void idCommonLocal::GuiFrameEvents() {
	if (game) {
		game->Shell_SyncWithSession();
	}
}
