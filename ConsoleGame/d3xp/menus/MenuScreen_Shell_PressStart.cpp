#include "MenuScreen.h"

/*
========================
idMenuScreen_Shell_PressStart::Initialize
========================
*/
void idMenuScreen_Shell_PressStart::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	startButton = std::make_shared<idMenuWidget_Button>();
	AddChild(startButton);
}

/*
========================
idMenuScreen_Shell_Root::Update
========================
*/
void idMenuScreen_Shell_PressStart::Update() {
	if (menuData) {
		std::shared_ptr<idMenuWidget_CommandBar> cmdBar = menuData->GetCmdBar();
		if (cmdBar) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->action.Set(widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED);
		}
	}

	idMenuScreen::Update();
}

/*
========================
idMenuScreen_Shell_PressStart::ShowScreen
========================
*/
void idMenuScreen_Shell_PressStart::ShowScreen() {

	if (startButton) {
		startButton->SetLabel("press start");
	}

	idMenuScreen::ShowScreen();
}

/*
========================
idMenuScreen_Shell_PressStart::HideScreen
========================
*/
void idMenuScreen_Shell_PressStart::HideScreen() {
	idMenuScreen::HideScreen();
}
