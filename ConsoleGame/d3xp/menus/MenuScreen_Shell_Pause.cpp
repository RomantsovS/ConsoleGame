#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

const static int NUM_PAUSE_OPTIONS = 6;

enum class pauseMenuCmds_t {
	PAUSE_CMD_RESTART,
	PAUSE_CMD_DEAD_RESTART,
	PAUSE_CMD_SETTINGS,
	PAUSE_CMD_EXIT,
	PAUSE_CMD_LEAVE,
	PAUSE_CMD_RETURN,
	PAUSE_CMD_LOAD,
	PAUSE_CMD_SAVE,
	PAUSE_CMD_PS3,
	PAUSE_CMD_INVITE_FRIENDS
};

/*
========================
idMenuScreen_Shell_Pause::Initialize
========================
*/
void idMenuScreen_Shell_Pause::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuPause");

	options = std::make_shared<idMenuWidget_DynamicList>();
	options->SetNumVisibleOptions(NUM_PAUSE_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);
	AddChild(options);

	while (options->GetChildren().size() < NUM_PAUSE_OPTIONS) {
		std::shared_ptr<idMenuWidget_Button> buttonWidget = std::make_shared<idMenuWidget_Button>();
		buttonWidget->Initialize(data);
		//buttonWidget->RegisterEventObserver(helpWidget);
		options->AddChild(buttonWidget);
	}
	options->Initialize(data);

	options->AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN));
	options->AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_UP));
	options->AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	options->AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE).Set(std::make_shared<idWidgetActionHandler>(options, actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE));
}

/*
========================
idMenuScreen_Shell_Pause::Update
========================
*/
void idMenuScreen_Shell_Pause::Update() {

	if (auto spMenuData = menuData.lock()) {
		std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
		if (cmdBar) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);

			std::shared_ptr<idPlayer> player = gameLocal.GetLocalPlayer();
			if (player) {
			}

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_RETURN));
		}
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_Pause::ShowScreen
========================
*/
void idMenuScreen_Shell_Pause::ShowScreen() {

	std::vector<std::vector<std::string>> menuOptions;
	std::vector<std::string> option;

	/*bool isDead = false;
	idPlayer* player = gameLocal.GetLocalPlayer();
	if (player != NULL) {
		if (player->health <= 0) {
			isDead = true;
		}
	}*/

	option.push_back("RETURN TO GAME");	// return to game
	menuOptions.push_back(option);
	option.clear();
	/*option.push_back("#str_02179");	// save game
	menuOptions.push_back(option);
	option.clear();
	option.push_back("#str_02187");	// load game
	menuOptions.push_back(option);
	option.clear();
	option.push_back("#str_swf_settings");	// settings
	menuOptions.push_back(option);
	option.clear();*/
	option.push_back("EXIT GAME");	// exit game
	menuOptions.push_back(option);

	int index = 0;
	std::shared_ptr<idMenuWidget_Button> buttonWidget = nullptr;
	options->GetChildByIndex(index)->ClearEventActions();
	options->GetChildByIndex(index)->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_RETURN));
	buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));
	index++;
	options->GetChildByIndex(index)->ClearEventActions();
	options->GetChildByIndex(index)->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_COMMAND, static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_EXIT));
	buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));

	options->SetListData(menuOptions);
	idMenuScreen::ShowScreen();

	if (options->GetFocusIndex() >= menuOptions.size()) {
		options->SetViewIndex(0);
		options->SetFocusIndex(0);
	}

}

/*
========================
idMenuScreen_Shell_Pause::HideScreen
========================
*/
void idMenuScreen_Shell_Pause::HideScreen() {
	idMenuScreen::HideScreen();
}

/*
========================
idMenuScreen_Shell_Pause::HandleExitGameBtn
========================
*/
void idMenuScreen_Shell_Pause::HandleExitGameBtn() {
	common->QuitRequest();
}

/*
========================
idMenuScreen_Shell_Pause::HandleRestartBtn
========================
*/
void idMenuScreen_Shell_Pause::HandleRestartBtn() {
	cmdSystem->AppendCommandText("restartMap\n");
}

/*
========================
idMenuScreen_Shell_Pause::HandleAction
========================
*/
bool idMenuScreen_Shell_Pause::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget>& widget, bool forceHandled) {

	auto spMenuData = menuData.lock();

	if (!spMenuData) {
		return true;
	}

	if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_ROOT)) {
		return false;
	}

	const widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_COMMAND: {
		switch (parms[0]->ToInteger()) {
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_RESTART) : {
				HandleRestartBtn();
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_DEAD_RESTART) : {
				cmdSystem->AppendCommandText("restartMap\n");
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_SETTINGS) : {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_SETTINGS);
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_LEAVE) :
				case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_EXIT) : {
				HandleExitGameBtn();
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_RETURN) : {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_INVALID);
				game->Shell_Show(false);
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_LOAD) : {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_LOAD);
				break;
			}
			case static_cast<int>(pauseMenuCmds_t::PAUSE_CMD_SAVE) : {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_SAVE);
				break;
			}
		}
		return true;
	}
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
