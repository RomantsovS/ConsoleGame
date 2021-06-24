#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

/*
========================
idMenuWidget_Button::Update
========================
*/
void idMenuWidget_Button::Update() {
	if (auto spMenuData = menuData.lock()) {
		if(spMenuData->GetGUI())
			BindSprite(spMenuData->GetGUI()->GetRootObject());
	}

	if (!GetSprite()) {
		return;
	}

	std::shared_ptr<idSWFScriptObject> spriteObject = GetSprite()->GetScriptObject();

	if (btnLabel.empty()) {
		if (values.size() > 0) {
			for (size_t val = 0; val < values.size(); ++val) {
				std::shared_ptr<idSWFScriptObject> textObject = spriteObject->GetNestedObj("txtVal");
				if (textObject) {
					std::shared_ptr<idSWFTextInstance> text = textObject->GetText();
					text->SetText(values[val].c_str());
				}
			}
		}
		else {
			ClearSprite();
		}
	}
	else {
		std::shared_ptr<idSWFScriptObject> textObject = spriteObject->GetNestedObj("label0", "txtVal");
		if (textObject) {
			std::shared_ptr<idSWFTextInstance> text = textObject->GetText();
			text->SetText(btnLabel.c_str());
		}
	}

	// events
	spriteObject->Set("onPress", idSWFScriptVar(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_PRESS, 0)));
	spriteObject->Set("onRelease", idSWFScriptVar(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_RELEASE, 0)));
}

/*
========================
idMenuWidget_Button::ExecuteEvent
========================
*/
bool idMenuWidget_Button::ExecuteEvent(const idWidgetEvent& event) {
	bool handled = false;

	// do nothing at all if it's disabled
	if (GetState() != widgetState_t::WIDGET_STATE_DISABLED) {
		switch (event.type) {
		case widgetEvent_t::WIDGET_EVENT_PRESS: {
			/*if (GetMenuData()) {
				GetMenuData()->PlaySound(GUI_SOUND_ADVANCE);
			}*/
			//AnimateToState(ANIM_STATE_DOWN);
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_RELEASE: {
			//AnimateToState(ANIM_STATE_UP);
			//GetMenuData()->ClearWidgetActionRepeater();
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_ROLL_OVER: {
			/*if (GetMenuData()) {
				GetMenuData()->PlaySound(GUI_SOUND_ROLL_OVER);
			}*/
			//AnimateToState(ANIM_STATE_OVER);
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_ROLL_OUT: {
			//AnimateToState(ANIM_STATE_UP);
			//GetMenuData()->ClearWidgetActionRepeater();
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_FOCUS_OFF: {
			SetState(widgetState_t::WIDGET_STATE_NORMAL);
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_FOCUS_ON: {
			SetState(widgetState_t::WIDGET_STATE_SELECTING);
			handled = true;
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_SCROLL_LEFT_RELEASE: {
			if(auto menuData_sp = GetMenuData().lock())
				menuData_sp->ClearWidgetActionRepeater();
			break;
		}
		case widgetEvent_t::WIDGET_EVENT_SCROLL_RIGHT_RELEASE: {
			if (auto menuData_sp = GetMenuData().lock())
				menuData_sp->ClearWidgetActionRepeater();
			break;
		}
		}
	}

	idMenuWidget::ExecuteEvent(event);

	return handled;
}

/*
========================
idMenuWidget_Button::AddValue
========================
*/
void idMenuWidget_Button::SetValues(std::vector<std::string>& list) {
	values.clear();
	for (size_t i = 0; i < list.size(); ++i) {
		values.push_back(list[i]);
	}
}
