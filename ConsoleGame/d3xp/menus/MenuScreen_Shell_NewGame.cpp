#pragma hdrstop
#include <precompiled.h>
#include "../Game_local.h"

const static int NUM_NEW_GAME_OPTIONS = 8;
/*
========================
idMenuScreen_Shell_NewGame::Initialize
========================
*/
void idMenuScreen_Shell_NewGame::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	if (data != NULL) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuNewGame");

	options = std::make_shared<idMenuWidget_DynamicList>();
	std::vector<std::vector<std::string>> menuOptions;
	std::vector<std::string> option;
	option.push_back("snake");
	menuOptions.push_back(option);

	option[0] = "bomber";
	menuOptions.push_back(option);

	options->SetListData(menuOptions);
	options->SetNumVisibleOptions(NUM_NEW_GAME_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);

	while (options->GetChildren().size() < NUM_NEW_GAME_OPTIONS) {
		std::shared_ptr<idMenuWidget_Button> buttonWidget = std::make_shared<idMenuWidget_Button>();
		buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED, options->GetChildren().size());
		buttonWidget->Initialize(data);
		options->AddChild(buttonWidget);
	}
	options->Initialize(data);

	AddChild(options);

	btnBack = std::make_shared<idMenuWidget_Button>();
	btnBack->Initialize(data);
	btnBack->SetLabel("CAMPAIGN");
	btnBack->SetSpritePath(GetSpritePath(), "info", "btnBack");
	btnBack->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_GO_BACK);

	AddChild(btnBack);

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
idMenuScreen_Shell_NewGame::Update
========================
*/
void idMenuScreen_Shell_NewGame::Update() {

	if (auto spMenuData = menuData.lock()) {
		std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
		if (cmdBar) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY2);
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_GO_BACK);

			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();
	if (BindSprite(root)) {
		std::shared_ptr<idSWFTextInstance> heading = GetSprite()->GetScriptObject()->GetNestedText("info", "txtHeading");
		if (heading) {
			heading->SetText("NEW GAME");	// NEW GAME
		}
	}

	if (btnBack) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_NewGame::ShowScreen
========================
*/
void idMenuScreen_Shell_NewGame::ShowScreen() {
	idMenuScreen::ShowScreen();
}

/*
========================
idMenuScreen_Shell_NewGame::HideScreen
========================
*/
void idMenuScreen_Shell_NewGame::HideScreen() {
	idMenuScreen::HideScreen();
}

/*
========================
idMenuScreen_Shell_NewGame::HandleAction h
========================
*/
bool idMenuScreen_Shell_NewGame::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget>& widget, bool forceHandled) {

	if (auto spMenuData = menuData.lock()) {
		if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_NEW_GAME)) {
			return false;
		}
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_GO_BACK: {
		if (auto spMenuData = menuData.lock()) {
			spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_CAMPAIGN);
		}
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED: {
		if (options == NULL) {
			return true;
		}

		int selectionIndex = options->GetViewIndex();
		if (parms.size() == 1) {
			selectionIndex = parms[0]->ToInteger();
		}

		if (selectionIndex != options->GetFocusIndex()) {
			options->SetViewIndex(selectionIndex);
			options->SetFocusIndex(selectionIndex);
		}

		if (auto spMenuData = menuData.lock()) {
			std::shared_ptr<idMenuHandler_Shell> shell = std::dynamic_pointer_cast<idMenuHandler_Shell>(spMenuData);
			if (shell) {
				shell->SetNewGameType(0);
				shell->StartGame(selectionIndex);
			}
		}

		return true;
	}
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
