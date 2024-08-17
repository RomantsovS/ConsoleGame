
#include "idlib/precompiled.h"
#include "../Game_local.h"
#include "../../framework/Common_local.h"

/*
========================
idMenuScreen_Shell_PressStart::Initialize
========================
*/
void idMenuScreen_Shell_PressStart::Initialize(std::shared_ptr<idMenuHandler> data) noexcept {
	idMenuScreen::Initialize(data);

	if (data) {
		menuGUI = data->GetGUI();
	}

	SetSpritePath("menuStart");
}

/*
========================
idMenuScreen_Shell_Root::Update
========================
*/
void idMenuScreen_Shell_PressStart::Update() noexcept {
	if (auto spMenuData = menuData.lock()) {
		std::shared_ptr<idMenuWidget_CommandBar> cmdBar = spMenuData->GetCmdBar();
		if (cmdBar) {
			cmdBar->ClearAllButtons();
			idMenuWidget_CommandBar::buttonInfo_t* buttonInfo;
			buttonInfo = cmdBar->GetButton(idMenuWidget_CommandBar::BUTTON_JOY1);
			buttonInfo->label = "press start";
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

/*
========================
idMenuScreen_Shell_PressStart::HandleAction
========================
*/
bool idMenuScreen_Shell_PressStart::HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled) {

	auto spMenuData = menuData.lock();

	if (!spMenuData) {
		return true;
	}

	if (spMenuData->ActiveScreen() != static_cast<int>(shellAreas_t::SHELL_AREA_START)) {
		return false;
	}

	widgetAction_t actionType = action.GetType();

	switch (actionType) {
	case widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED: {
		session->RegisterLocalUser();

		return true;
	}
	}

	return idMenuWidget::HandleAction(action, event, widget, forceHandled);
}
