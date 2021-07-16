#pragma hdrstop
#include <precompiled.h>
#include "../Game_local.h"

const static int NUM_MAIN_OPTIONS = 6;

/*
========================
idMenuScreen_Shell_Root::Initialize
========================
*/
void idMenuScreen_Shell_Root::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuMain");

	options = std::make_shared<idMenuWidget_DynamicList>();
	options->SetNumVisibleOptions(NUM_MAIN_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->Initialize(data);
	options->SetWrappingAllowed(true);
	AddChild(options);

	while (options->GetChildren().size() < NUM_MAIN_OPTIONS) {
		std::shared_ptr<idMenuWidget_Button> const buttonWidget = std::make_shared<idMenuWidget_Button>();
		buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED, options->GetChildren().size());
		buttonWidget->Initialize(data);
		//buttonWidget->RegisterEventObserver(helpWidget);
		options->AddChild(buttonWidget);
	}
	/*
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK));
	options->AddEventAction(WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE));
	options->AddEventAction(WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE).Set(new (TAG_SWF) idWidgetActionHandler(options, WIDGET_ACTION_EVENT_STOP_REPEATER, WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE));
	*/

	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT).Set(std::make_shared<idWidgetActionHandler>(shared_from_this(), actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_RIGHT_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT_RELEASE).Set(std::make_shared<idWidgetActionHandler>(shared_from_this(), actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT_RELEASE));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT).Set(std::make_shared<idWidgetActionHandler>(shared_from_this(), actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_LEFT_START_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT_RELEASE).Set(std::make_shared<idWidgetActionHandler>(shared_from_this(), actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER,
		widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT_RELEASE));
	AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED, 0);

}

/*
========================
idMenuScreen_Shell_Root::Update
========================
*/
void idMenuScreen_Shell_Root::Update() {

	if (auto spMenuData = menuData.lock()) {
		std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
		if (cmdBar) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;

			if (false) {
				buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
				buttonInfo->label = "back";
				buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_GO_BACK);
			}

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->label = "";
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idMenuScreen::Update();
}

enum rootMenuCmds_t {
	ROOT_CMD_START_DEMO,
	ROOT_CMD_START_DEMO2,
	ROOT_CMD_SETTINGS,
	ROOT_CMD_QUIT,
	ROOT_CMD_DEV,
	ROOT_CMD_CAMPAIGN,
	ROOT_CMD_MULTIPLAYER,
	ROOT_CMD_PLAYSTATION,
	ROOT_CMD_CREDITS
};

/*
========================
idMenuScreen_Shell_Root::ShowScreen
========================
*/
void idMenuScreen_Shell_Root::ShowScreen() {

	std::vector<std::vector<std::string>> menuOptions;
	options->SetListData(menuOptions);

	idMenuScreen::ShowScreen();

	if (auto spMenuData = menuData.lock()) {
		std::shared_ptr<idMenuHandler_Shell> shell = std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
		if (shell) {
			std::shared_ptr<idMenuWidget_MenuBar> menuBar = shell->GetMenuBar();
			if (menuBar) {
				menuBar->SetFocusIndex(GetRootIndex());
			}
		}
	}
}

/*
========================
idMenuScreen_Shell_Root::HideScreen
========================
*/
void idMenuScreen_Shell_Root::HideScreen() {
	idMenuScreen::HideScreen();
}

/*
========================
idMenuScreen_Shell_Root::HandleExitGameBtn
========================
*/
void idMenuScreen_Shell_Root::HandleExitGameBtn() {
	common->QuitRequest();
}

/*
========================
idMenuScreen_Shell_Root::GetRootIndex
========================
*/
int idMenuScreen_Shell_Root::GetRootIndex() {
	if (options) {
		return options->GetFocusIndex();
	}

	return 0;
}

/*
========================
idMenuScreen_Shell_Root::SetRootIndex
========================
*/
void idMenuScreen_Shell_Root::SetRootIndex(int index) {
	if (options) {
		options->SetFocusIndex(index);
	}
}

/*
========================
idMenuScreen_Shell_Root::HandleAction
========================
*/
bool idMenuScreen_Shell_Root::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget>& widget, bool forceHandled) {

	auto spMenuData = menuData.lock();
	if (!spMenuData) {
		return true;
	}

	if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_ROOT)) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_GO_BACK: {
		session->MoveToPressStart();
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED: {
		if (true) {

			auto shell = std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
			if (!shell) {
				return true;
			}

			std::shared_ptr<idMenuWidget_MenuBar> menuBar = shell->GetMenuBar();

			if (!menuBar) {
				return true;
			}

			const std::shared_ptr<idMenuWidget_MenuButton> buttonWidget = std::dynamic_pointer_cast<idMenuWidget_MenuButton>(menuBar->GetChildByIndex(menuBar->GetFocusIndex()));
			if (!buttonWidget) {
				return true;
			}

			idWidgetEvent pressEvent(widgetEvent_t::WIDGET_EVENT_PRESS, 0, nullptr, idSWFParmList());
			menuBar->ReceiveEvent(pressEvent);
			return true;
		}
		break;
	}
	case widgetAction_t::WIDGET_ACTION_SCROLL_HORIZONTAL: {

		auto shell = std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
		if (!shell) {
			return true;
		}

		std::shared_ptr<idMenuWidget_MenuBar> menuBar = shell->GetMenuBar();

		if (!menuBar) {
			return true;
		}

		int index = menuBar->GetViewIndex();
		const int dir = parms[0]->ToInteger();

		const int totalCount = menuBar->GetTotalNumberOfOptions();

		index += dir;
		if (index < 0) {
			index = totalCount - 1;
		}
		else if (index >= totalCount) {
			index = 0;
		}

		SetRootIndex(index);
		menuBar->SetViewIndex(index);
		menuBar->SetFocusIndex(index);

		return true;
	}
	case widgetAction_t::WIDGET_ACTION_COMMAND: {
		switch (parms[0]->ToInteger()) {
		case ROOT_CMD_SETTINGS: {
			spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_SETTINGS);
			break;
		}
		case ROOT_CMD_QUIT: {
			HandleExitGameBtn();
			break;
		}
		case ROOT_CMD_DEV: {
			spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_DEV);
			break;
		}
		case ROOT_CMD_CAMPAIGN: {
			spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_CAMPAIGN);
			break;
		}
		}
		return true;
	}
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
