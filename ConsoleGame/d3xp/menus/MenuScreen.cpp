#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

idMenuScreen::idMenuScreen() :menuGUI(nullptr) {
}

idMenuScreen::~idMenuScreen() {
}

/*
========================
idMenuScreen::Update
========================
*/
void idMenuScreen::Update() {
	if (!menuGUI) {
		return;
	}

	//
	// Display
	//
	for (size_t childIndex = 0; childIndex < GetChildren().size(); ++childIndex) {
		GetChildByIndex(childIndex)->Update();
		if (GetChildByIndex(childIndex)->GetSprite())
			GetChildByIndex(childIndex)->GetSprite()->SetVisible(true);
	}

	if (menuData) {
		menuData->UpdateChildren();
	}
}

/*
========================
idMenuScreen::UpdateCmds
========================
*/
void idMenuScreen::UpdateCmds() {
	const std::shared_ptr<idSWF> gui = menuGUI;

	std::shared_ptr<idSWFScriptObject> const shortcutKeys = gui->GetGlobal("shortcutKeys").GetObjectScript();
	/*if (!verify(shortcutKeys != NULL)) {
		return;
	}*/

	idSWFScriptVar clearFunc = shortcutKeys->Get("clear");
	if (clearFunc.IsFunction()) {
		clearFunc.GetFunction()->Call(nullptr, idSWFParmList());
	}

	// NAVIGATION: UP/DOWN, etc.
	const std::shared_ptr<idSWFScriptObject> buttons = gui->GetRootObject()->GetObjectScript("buttons");
	if (buttons) {
		std::shared_ptr<idSWFScriptObject> const btnUp = buttons->GetObjectScript("btnUp");
		if (btnUp) {
			btnUp->Set("onPress", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_UP, 0)));
			btnUp->Set("onRelease", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_UP_RELEASE, 0)));
			shortcutKeys->Set("UP", btnUp);
		}

		std::shared_ptr<idSWFScriptObject> const btnDown = buttons->GetObjectScript("btnDown");
		if (btnDown) {
			btnDown->Set("onPress", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN, 0)));
			btnDown->Set("onRelease", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_DOWN_RELEASE, 0)));
			shortcutKeys->Set("DOWN", btnDown);
		}

		std::shared_ptr<idSWFScriptObject> const btnLeft = buttons->GetObjectScript("btnLeft");
		if (btnLeft) {
			btnLeft->Set("onPress", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT, 0)));
			btnLeft->Set("onRelease", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT_RELEASE, 0)));
			shortcutKeys->Set("LEFT", btnLeft);
		}

		std::shared_ptr<idSWFScriptObject> const btnRight = buttons->GetObjectScript("btnRight");
		if (btnRight) {
			btnRight->Set("onPress", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT, 0)));
			btnRight->Set("onRelease", static_cast<idSWFScriptVar>(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT_RELEASE, 0)));
			shortcutKeys->Set("RIGHT", btnRight);
		}
	}
}

/*
========================
idMenuScreen::HideScreen
========================
*/
void idMenuScreen::HideScreen() {
	if (GetSprite()) {
		GetSprite()->SetVisible(false);
	}

	if (!menuGUI) {
		return;
	}

	if (!BindSprite(menuGUI->GetRootObject())) {
		return;
	}

	GetSprite()->SetVisible(false);

	Update();
}

/*
========================
idMenuScreen::ShowScreen
========================
*/
void idMenuScreen::ShowScreen() {
	if (!menuGUI) {
		return;
	}

	if (!BindSprite(menuGUI->GetRootObject())) {
		return;
	}

	GetSprite()->SetVisible(true);

	Update();

	SetFocusIndex(GetFocusIndex(), true);

	if (GetSprite()) {
		GetSprite()->SetVisible(true);
	}
}
