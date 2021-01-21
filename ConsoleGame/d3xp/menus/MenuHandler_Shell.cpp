#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

static const int MAX_MENU_OPTIONS = 6;

void idMenuHandler_Shell::Update() {

	if (!gui || !gui->IsActive()) {
		return;
	}

	if (nextState != state) {
		if (nextState == shellState_t::SHELL_STATE_PRESS_START) {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_START);
			state = nextState;
			if (menuBar && gui) {
				menuBar->ClearSprite();
			}
		}
		else if (nextState == shellState_t::SHELL_STATE_IDLE) {
			if (nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_START) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_PARTY_LOBBY) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_GAME_LOBBY) || nextScreen == static_cast<int>(shellAreas_t::SHELL_AREA_INVALID)) {
				nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_ROOT);
			}
			if (menuBar && gui) {
				std::shared_ptr<idSWFScriptObject> root = gui->GetRootObject();
				menuBar->BindSprite(root);
				SetupPCOptions();
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

			if (cmdBar) {
				cmdBar->ClearAllButtons();
				cmdBar->Update();
			}
		}
		else {

			if (activeScreen > static_cast<int>(shellAreas_t::SHELL_AREA_INVALID) && activeScreen < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS)) {
				if (menuScreens[activeScreen] && menuScreens[nextScreen])
					menuScreens[activeScreen]->HideScreen();
				else {
					nextScreen = activeScreen;
					common->Error("menu screen %d is not implemented", activeScreen);
					return;
				}
			}

			if (nextScreen > static_cast<int>(shellAreas_t::SHELL_AREA_INVALID) && nextScreen < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS)) {
				if (menuScreens[nextScreen]) {
					menuScreens[nextScreen]->UpdateCmds();
					menuScreens[nextScreen]->ShowScreen();
				}
				else
					common->Error("menu screen %d is not implemented", nextScreen);

			}
		}
		activeScreen = nextScreen;
	}

	if (cmdBar && cmdBar->GetSprite()) {
		cmdBar->GetSprite()->SetVisible(true);
	}

	idMenuHandler::Update();
}

/*
========================
idMenuHandler_Shell::HandleGuiEvent
========================
*/
bool idMenuHandler_Shell::HandleGuiEvent(const sysEvent_t* sev) {
	return idMenuHandler::HandleGuiEvent(sev);
}

/*
========================
idMenuHandler_Shell::Initialize
========================
*/
void idMenuHandler_Shell::Initialize(const std::string& filename) {
	idMenuHandler::Initialize(filename);

	//---------------------
	// Initialize the menus
	//---------------------
#define BIND_SHELL_SCREEN( screenId, className, menuHandler )	\
	menuScreens[ (screenId) ] = std::make_shared<className>();	\
	menuScreens[ (screenId) ]->Initialize( menuHandler );

	for (int i = 0; i < static_cast<int>(shellAreas_t::SHELL_NUM_AREAS); ++i) {
		menuScreens[i] = nullptr;
	}

	if (inGame) {
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_ROOT), idMenuScreen_Shell_Pause, shared_from_this());
	}
	else {
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_START), idMenuScreen_Shell_PressStart, shared_from_this());
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_ROOT), idMenuScreen_Shell_Root, shared_from_this());
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_CAMPAIGN), idMenuScreen_Shell_Singleplayer, shared_from_this());
		BIND_SHELL_SCREEN(static_cast<int>(shellAreas_t::SHELL_AREA_NEW_GAME), idMenuScreen_Shell_NewGame, shared_from_this());
	}

	menuBar = std::make_shared<idMenuWidget_MenuBar>();
	menuBar->SetSpritePath("pcBar");
	menuBar->Initialize(shared_from_this());
	menuBar->SetNumVisibleOptions(MAX_MENU_OPTIONS);
	menuBar->SetWrappingAllowed(true);
	menuBar->SetButtonSpacing(45.0f);
	while (menuBar->GetChildren().size() < MAX_MENU_OPTIONS) {
		std::shared_ptr<idMenuWidget_MenuButton> const navButton = std::make_shared<idMenuWidget_MenuButton>();
		std::shared_ptr<idMenuScreen_Shell_Root> rootScreen = std::dynamic_pointer_cast<idMenuScreen_Shell_Root>(menuScreens[static_cast<int>(shellAreas_t::SHELL_AREA_ROOT)]);
		if (rootScreen) {
			//navButton->RegisterEventObserver(rootScreen->GetHelpWidget());
		}
		menuBar->AddChild(navButton);
	}
	AddChild(menuBar);

	//
	// command bar
	//
	cmdBar = std::make_shared<idMenuWidget_CommandBar>();
	cmdBar->SetSpritePath("prompts");
	cmdBar->Initialize(shared_from_this());
	AddChild(cmdBar);
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

	if (show && gui && gui->IsActive()) {
		return;
	}
	else if (!show && gui && !gui->IsActive()) {
		return;
	}

	idMenuHandler::ActivateMenu(show);
	if (show) {

		SetupPCOptions();

	}
	else {
		nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_INVALID);
		activeScreen = static_cast<int>(shellAreas_t::SHELL_AREA_INVALID);
		nextState = shellState_t::SHELL_STATE_INVALID;
		state = shellState_t::SHELL_STATE_INVALID;
	}
}

enum class shellCommandsPC_t {
	SHELL_CMD_DEMO0,
	SHELL_CMD_DEMO1,
	SHELL_CMD_DEV,
	SHELL_CMD_CAMPAIGN,
	SHELL_CMD_MULTIPLAYER,
	SHELL_CMD_SETTINGS,
	SHELL_CMD_CREDITS,
	SHELL_CMD_QUIT
};

/*
========================
idMenuHandler_Shell::SetPCOptionsVisible
========================
*/
void idMenuHandler_Shell::SetupPCOptions() {

	if (inGame) {
		return;
	}

	navOptions.clear();

	if (menuBar) {
		//navOptions.push_back("DEV");	// DEV
		navOptions.push_back("campaign");	// singleplayer
		//navOptions.push_back("settings");	// settings
		navOptions.push_back("quit");	// quit


		std::shared_ptr<idMenuWidget_MenuButton> buttonWidget = nullptr;
		int index = 0;

		/*buttonWidget = std::dynamic_pointer_cast<idMenuWidget_MenuButton>(menuBar->GetChildByIndex(index));
		if (buttonWidget) {
			buttonWidget->ClearEventActions();
			buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(shellCommandsPC_t::SHELL_CMD_DEV), index);
		}
		index++;*/
		buttonWidget = std::dynamic_pointer_cast<idMenuWidget_MenuButton>(menuBar->GetChildByIndex(index));
		if (buttonWidget) {
			buttonWidget->ClearEventActions();
			buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(shellCommandsPC_t::SHELL_CMD_CAMPAIGN), index);
		}
		index++;
		/*buttonWidget = std::dynamic_pointer_cast<idMenuWidget_MenuButton>(menuBar->GetChildByIndex(index));
		if (buttonWidget) {
			buttonWidget->ClearEventActions();
			buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(shellCommandsPC_t::SHELL_CMD_SETTINGS), index);
		}
		index++;*/
		buttonWidget = std::dynamic_pointer_cast<idMenuWidget_MenuButton>(menuBar->GetChildByIndex(index));
		if (buttonWidget) {
			buttonWidget->ClearEventActions();
			buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(shellCommandsPC_t::SHELL_CMD_QUIT), index);
		}
	}

	if (menuBar && gui) {
		std::shared_ptr<idSWFScriptObject> root = gui->GetRootObject();
		if (menuBar->BindSprite(root)) {
			menuBar->GetSprite()->SetVisible(true);
			menuBar->SetListHeadings(navOptions);
			menuBar->Update();

			std::shared_ptr<idMenuScreen_Shell_Root> menu = std::dynamic_pointer_cast<idMenuScreen_Shell_Root>(menuScreens[static_cast<int>(shellAreas_t::SHELL_AREA_ROOT)]);
			if (menu) {
				const int activeIndex = menu->GetRootIndex();
				menuBar->SetViewIndex(activeIndex);
				menuBar->SetFocusIndex(activeIndex);
			}
		}
	}
}

/*
========================
idMenuHandler_Shell::HandleExitGameBtn
========================
*/
void idMenuHandler_Shell::HandleExitGameBtn() {
	common->Quit();
}

/*
========================
idMenuHandler_Shell::HandleAction
========================
*/
bool idMenuHandler_Shell::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled) {

	if (activeScreen == static_cast<int>(shellAreas_t::SHELL_AREA_INVALID)) {
		return true;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	if (event.type == widgetEvent_t::WIDGET_EVENT_COMMAND) {
		if (menuScreens[activeScreen] && !forceHandled) {
			if (menuScreens[activeScreen]->HandleAction(action, event, widget, true)) {
				if (actionType == widgetAction_t::WIDGET_ACTION_GO_BACK) {
					//PlaySound(GUI_SOUND_BACK);
				}
				else {
					//PlaySound(GUI_SOUND_ADVANCE);
				}
				return true;
			}
		}
	}

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_COMMAND: {
		int cmd = parms[0]->ToInteger();

		const int index = parms[1]->ToInteger();
		menuBar->SetFocusIndex(index);
		menuBar->SetViewIndex(index);

		std::shared_ptr<idMenuScreen_Shell_Root> menu = std::dynamic_pointer_cast<idMenuScreen_Shell_Root>(menuScreens[static_cast<int>(shellAreas_t::SHELL_AREA_ROOT)]);
		if (menu) {
			menu->SetRootIndex(index);
		}

		switch (cmd) {
		case static_cast<int>(shellCommandsPC_t::SHELL_CMD_DEV): {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_DEV);
			break;
		}
		case static_cast<int>(shellCommandsPC_t::SHELL_CMD_CAMPAIGN): {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_CAMPAIGN);
			break;
		}
		case static_cast<int>(shellCommandsPC_t::SHELL_CMD_SETTINGS): {
			nextScreen = static_cast<int>(shellAreas_t::SHELL_AREA_SETTINGS);
			break;
		}
		case static_cast<int>(shellCommandsPC_t::SHELL_CMD_QUIT): {
			HandleExitGameBtn();
			break;
		}
		}

		return true;
	}
	}

	return idMenuHandler::HandleAction(action, event, widget, forceHandled);
}

/*
========================
idMenuHandler_Shell::StartGame
========================
*/
void idMenuHandler_Shell::StartGame(int index) {
	if (index == 0) {
		cmdSystem->AppendCommandText(va("map %s %d\n", "map_name", 0));
	}
}
