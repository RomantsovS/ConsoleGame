#pragma hdrstop
#include "idlib/precompiled.h"
#include "../Game_local.h"

static const std::string BUTTON_NAMES[] = {
	"joy1",
	"joy2",
	"joy3",
	"joy4",
	"joy10",
	"tab"
};

/*
========================
idMenuWidget_CommandBar::ClearAllButtons
========================
*/
void idMenuWidget_CommandBar::ClearAllButtons() noexcept {
	for (int index = 0; index < MAX_BUTTONS; ++index) {
		buttons[index].label.clear();
		buttons[index].action.Set(widgetAction_t::WIDGET_ACTION_NONE);
	}
}

/*
========================
idMenuWidget_CommandBar::Update
========================
*/
void idMenuWidget_CommandBar::Update() noexcept {

	if (!GetSWFObject()) {
		return;
	}

	std::shared_ptr<idSWFScriptObject> root = GetSWFObject()->GetRootObject();

	if (!BindSprite(root.get())) {
		return;
	}

	// Setup the button order.
	std::vector<button_t> buttonOrder(MAX_BUTTONS);
	for (size_t i = 0; i < buttonOrder.size(); ++i) {
		buttonOrder[i] = static_cast<button_t>(i);
	}

	// FIXME: handle animating in of the button bar?
	GetSprite()->SetVisible(true);

	std::string shortcutName;
	for (size_t i = 0; i < buttonOrder.size(); ++i) {
		const std::string& buttonName = BUTTON_NAMES[buttonOrder[i]];

		std::shared_ptr<idSWFSpriteInstance> const buttonSprite = GetSprite()->GetScriptObject()->GetSprite(buttonName);
		if (!buttonSprite) {
			continue;
		}
		std::shared_ptr<idSWFTextInstance> const buttonText = buttonSprite->GetScriptObject()->GetText("txt_info");
		if (!buttonText) {
			continue;
		}

		if (buttons[i].action.GetType() != widgetAction_t::WIDGET_ACTION_NONE) {
			std::shared_ptr<idSWFScriptObject> shortcutKeys = GetSWFObject()->GetGlobal("shortcutKeys").GetObjectScript();
			if (shortcutKeys) {
				buttonSprite->GetScriptObject()->Set("onPress",
					idSWFScriptVar(std::make_shared<WrapWidgetSWFEvent>(shared_from_this(), widgetEvent_t::WIDGET_EVENT_COMMAND, i)));

				// bind the main action - need to use all caps here because shortcuts are stored that way
				shortcutName = buttonName;
				//shortcutName.ToUpper();
				shortcutKeys->Set(shortcutName, buttonSprite->GetScriptObject());

				// Some other keys have additional bindings. Remember that the button here is
				// actually the virtual button, and the physical button could be swapped based
				// on the UseCircleForAccept business on JPN PS3.
				switch (i) {
				case BUTTON_JOY1: {
					shortcutKeys->Set("ENTER", buttonSprite->GetScriptObject());
					break;
				}
				case BUTTON_JOY2: {
					shortcutKeys->Set("ESCAPE", buttonSprite->GetScriptObject());
					shortcutKeys->Set("BACKSPACE", buttonSprite->GetScriptObject());
					break;
				}
				case BUTTON_TAB: {
					shortcutKeys->Set("K_TAB", buttonSprite->GetScriptObject());
					break;
				}
				}
			}

			if (buttons[i].label.empty()) {
				buttonSprite->SetVisible(false);
			}
			else {
				buttonSprite->SetVisible(true);
				//buttonSprite->SetXPos(xPos);
				buttonText->SetText(buttons[i].label);
			}
		}
		else {
			buttonSprite->SetVisible(false);
			std::shared_ptr<idSWFScriptObject> const shortcutKeys = GetSWFObject()->GetGlobal("shortcutKeys").GetObjectScript();
			if (shortcutKeys) {
				buttonSprite->GetScriptObject()->Set("onPress", 0);
				// bind the main action - need to use all caps here because shortcuts are stored that way
				shortcutName = buttonName;
				//shortcutName.ToUpper();
				shortcutKeys->Set(shortcutName, buttonSprite->GetScriptObject());
			}
		}
	}
}

/*
========================
idMenuWidget_CommandBar::ReceiveEvent
========================
*/
bool idMenuWidget_CommandBar::ExecuteEvent(const idWidgetEvent& event) {
	if (event.type == widgetEvent_t::WIDGET_EVENT_COMMAND) {
		if (event.arg >= 0 && event.arg < buttons.size()) {
			auto this_sp = shared_from_this();
			HandleAction(buttons[event.arg].action, event, this_sp.get());
		}
		return true;
	}
	else {
		return idMenuWidget::ExecuteEvent(event);
	}
}
