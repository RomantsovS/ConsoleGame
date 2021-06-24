#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

const static int NUM_SINGLEPLAYER_OPTIONS = 8;
/*
========================
idMenuScreen_Shell_Singleplayer::Initialize
========================
*/
void idMenuScreen_Shell_Singleplayer::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuCampaign");

	options = std::make_shared<idMenuWidget_DynamicList>();
	options->SetNumVisibleOptions(NUM_SINGLEPLAYER_OPTIONS);
	options->SetSpritePath(GetSpritePath(), "info", "options");
	options->SetWrappingAllowed(true);
	AddChild(options);

	while (options->GetChildren().size() < NUM_SINGLEPLAYER_OPTIONS) {
		std::shared_ptr<idMenuWidget_Button> buttonWidget = std::make_shared<idMenuWidget_Button>();
		buttonWidget->AddEventAction(widgetEvent_t::WIDGET_EVENT_PRESS).Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED, options->GetChildren().size());
		//buttonWidget->RegisterEventObserver(helpWidget);
		buttonWidget->Initialize(data);
		options->AddChild(buttonWidget);
	}
	options->Initialize(data);

	btnBack = std::make_shared<idMenuWidget_Button>();
	btnBack->Initialize(data);
	btnBack->SetLabel("MAIN MENU");
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
idMenuScreen_Shell_Singleplayer::Update
========================
*/
void idMenuScreen_Shell_Singleplayer::Update() {

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
			heading->SetText("CAMPAIGN");
		}
	}

	if (btnBack) {
		btnBack->BindSprite(root);
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_Singleplayer::ShowScreen
========================
*/
void idMenuScreen_Shell_Singleplayer::ShowScreen() {

	std::vector<std::vector<std::string>> menuOptions;
	std::vector<std::string> option;

	/*canContinue = false;
	const saveGameDetailsList_t& saveGameInfo = session->GetSaveGameManager().GetEnumeratedSavegames();
	canContinue = (saveGameInfo.Num() > 0);
	if (canContinue) {
		option.Append("#str_swf_continue_game");	// continue game
		menuOptions.Append(option);
		option.Clear();
		option.Append("#str_01866");	// new game
		menuOptions.Append(option);
		option.Clear();
		option.Append("#str_01867");	// load game
		menuOptions.Append(option);

		int index = 0;
		idMenuWidget_Button* buttonWidget = dynamic_cast<idMenuWidget_Button*>(&options->GetChildByIndex(index));
		if (buttonWidget != NULL) {
			buttonWidget->SetDescription("#str_swf_continue_desc");
		}
		index++;
		buttonWidget = dynamic_cast<idMenuWidget_Button*>(&options->GetChildByIndex(index));
		if (buttonWidget != NULL) {
			buttonWidget->SetDescription("#str_02209");
		}
		index++;
		buttonWidget = dynamic_cast<idMenuWidget_Button*>(&options->GetChildByIndex(index));
		if (buttonWidget != NULL) {
			buttonWidget->SetDescription("#str_02213");
		}
		index++;

	}
	else {*/
		option.push_back("new game");	// new game
		menuOptions.push_back(option);
		option.clear();
		option.push_back("load game");	// load game
		menuOptions.push_back(option);

		if (options) {
			int index = 0;
			std::shared_ptr<idMenuWidget_Button> buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));
			if (buttonWidget) {
				//buttonWidget->SetDescription("start a new game");
			}
			index++;
			buttonWidget = std::dynamic_pointer_cast<idMenuWidget_Button>(options->GetChildByIndex(index));
			if (buttonWidget != NULL) {
				//buttonWidget->SetDescription("#str_02213");
			}
		}
	//}

	if (options) {
		options->SetListData(menuOptions);
	}

	idMenuScreen::ShowScreen();
}

/*
========================
idMenuScreen_Shell_Singleplayer::HideScreen
========================
*/
void idMenuScreen_Shell_Singleplayer::HideScreen() {
	idMenuScreen::HideScreen();
}

/*
========================
idMenuScreen_Shell_Singleplayer::ContinueGame
========================
*/
void idMenuScreen_Shell_Singleplayer::ContinueGame() {
	/*const saveGameDetailsList_t& saveGameInfo = session->GetSaveGameManager().GetEnumeratedSavegames();
	saveGameDetailsList_t sortedSaves = saveGameInfo;
	sortedSaves.Sort(idSort_SavesByDate());
	if (sortedSaves.Num() > 0) {
		if (sortedSaves[0].damaged) {
			class idSWFScriptFunction_ContinueDamaged : public idSWFScriptFunction_RefCounted {
			public:
				idSWFScriptVar Call(idSWFScriptObject* thisObject, const idSWFParmList& parms) {
					common->Dialog().ClearDialog(GDM_CORRUPT_CONTINUE);
					return idSWFScriptVar();
				}
			};

			idStaticList< idSWFScriptFunction*, 4 > callbacks;
			callbacks.Append(new (TAG_SWF) idSWFScriptFunction_ContinueDamaged());
			idStaticList< idStrId, 4 > optionText;
			optionText.Append(idStrId("#str_04339"));	// OK
			common->Dialog().AddDynamicDialog(GDM_CORRUPT_CONTINUE, callbacks, optionText, false, "");
		}
		else {
			const idStr& name = sortedSaves[0].slotName;
			cmdSystem->AppendCommandText(va("loadgame %s\n", name.c_str()));
		}
	}*/
}

/*
========================
idMenuScreen_Shell_Singleplayer::HandleAction
========================
*/
bool idMenuScreen_Shell_Singleplayer::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget>& widget, bool forceHandled) {

	auto spMenuData = menuData.lock();
	if (!spMenuData) {
		return true;
	}

	if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_CAMPAIGN)) {
		return false;
	}

	widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_GO_BACK: {
		spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_ROOT);
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED: {
		if (!options) {
			return true;
		}

		int selectionIndex = options->GetViewIndex();
		if (parms.size() == 1) {
			selectionIndex = parms[0]->ToInteger();
		}

		/*canContinue = false;
		const saveGameDetailsList_t& saveGameInfo = session->GetSaveGameManager().GetEnumeratedSavegames();
		canContinue = (saveGameInfo.Num() > 0);
		if (canContinue) {
			if (selectionIndex == 0) {
				ContinueGame();

			}
			else if (selectionIndex == 1) {
				class idSWFScriptFunction_NewGame : public idSWFScriptFunction_RefCounted {
				public:
					idSWFScriptFunction_NewGame(idMenuHandler* _menuData, bool _accept) {
						menuData = _menuData;
						accept = _accept;
					}
					idSWFScriptVar Call(idSWFScriptObject* thisObject, const idSWFParmList& parms) {
						common->Dialog().ClearDialog(GDM_DELETE_AUTOSAVE);
						if (accept) {
							menuData->SetNextScreen(SHELL_AREA_NEW_GAME, MENU_TRANSITION_SIMPLE);
						}
						return idSWFScriptVar();
					}
				private:
					idMenuHandler* menuData;
					bool accept;
				};
				common->Dialog().AddDialog(GDM_DELETE_AUTOSAVE, DIALOG_ACCEPT_CANCEL, new idSWFScriptFunction_NewGame(menuData, true), new idSWFScriptFunction_NewGame(menuData, false), true);
			}
			else if (selectionIndex == 2) {
				menuData->SetNextScreen(SHELL_AREA_LOAD, MENU_TRANSITION_SIMPLE);
			}
		}
		else {*/
			if (selectionIndex == 0) {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_NEW_GAME);
			}
			else if (selectionIndex == 1) {
				spMenuData->SetNextScreen(shellAreas_t::SHELL_AREA_LOAD);
			}
		//}

		return true;
	}
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
