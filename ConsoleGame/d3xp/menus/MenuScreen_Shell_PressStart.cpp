#include "MenuScreen.h"

/*
========================
idMenuScreen_Shell_PressStart::Initialize
========================
*/
void idMenuScreen_Shell_PressStart::Initialize(std::shared_ptr<idMenuHandler> data) {
	idMenuScreen::Initialize(data);

	startButton = std::make_shared<idMenuWidget_Button>();
	AddChild(startButton);
}

/*
========================
idMenuScreen_Shell_Root::Update
========================
*/
void idMenuScreen_Shell_PressStart::Update() {
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
