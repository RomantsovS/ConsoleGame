#pragma hdrstop
#include <precompiled.h>
#include "../Game_local.h"

/*
========================
idMenuWidget_NavButton::Update
========================
*/
void idMenuWidget_MenuButton::Update() {

	if (!GetSprite()) {
		return;
	}

	if (btnLabel.empty()) {
		GetSprite()->SetVisible(false);
		return;
	}

	GetSprite()->SetVisible(true);

	std::shared_ptr<idSWFScriptObject> const spriteObject = GetSprite()->GetScriptObject();
	std::shared_ptr<idSWFTextInstance> const text = spriteObject->GetNestedText("txtVal");
	if (text) {
		text->SetText(btnLabel.c_str());
	}

	GetSprite()->SetXPos(xPos);

	std::shared_ptr<idSWFScriptObject> textObj = spriteObject->GetNestedObj("txtVal");

	if (textObj) {
#ifdef DEBUG
		textObj->Set("onPress", idSWFScriptVar(std::shared_ptr<WrapWidgetSWFEvent>(DBG_NEW WrapWidgetSWFEvent(shared_from_this(), widgetEvent_t::WIDGET_EVENT_PRESS, 0))));
		textObj->Set("onRelease", idSWFScriptVar(std::shared_ptr<WrapWidgetSWFEvent>(DBG_NEW WrapWidgetSWFEvent(shared_from_this(), widgetEvent_t::WIDGET_EVENT_RELEASE, 0))));
#else
		textObj->Set("onPress", idSWFScriptVar(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_PRESS, 0)));
		textObj->Set("onRelease", idSWFScriptVar(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_RELEASE, 0)));
#endif
	}
}
