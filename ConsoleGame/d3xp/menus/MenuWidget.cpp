#pragma hdrstop
#include "../../idLib/precompiled.h"
#include "../Game_local.h"

/*
========================
idMenuWidget::idMenuWidget
========================
*/
idMenuWidget::idMenuWidget() :
	boundSprite(nullptr),
	focusIndex(0),
	widgetState(widgetState_t::WIDGET_STATE_NORMAL),
	menuData(nullptr),
	swfObj(nullptr) {
	parent.reset();

	eventActionLookup.resize(static_cast<int>(widgetEvent_t::MAX_WIDGET_EVENT));
	for (size_t i = 0; i < eventActionLookup.size(); ++i) {
		eventActionLookup[i] = INVALID_ACTION_INDEX;
	}
}

/*
========================
idMenuWidget::~idMenuWidget
========================
*/
idMenuWidget::~idMenuWidget() {
	Cleanup();
}

void idMenuWidget::Cleanup() {
	// free all children
	for (size_t i = 0; i < children.size(); ++i) {
		children[i] = nullptr;
	}

	children.clear();
}

/*
========================
idMenuWidget::AddChild
========================
*/
void idMenuWidget::AddChild(std::shared_ptr<idMenuWidget> widget) {
	if (widget->GetParent()) {
		// take out of previous parent
		widget->GetParent()->RemoveChild(widget);
	}

	widget->SetParent(shared_from_this());
	children.push_back(widget);
}

/*
========================
idMenuWidget::RemoveChild
========================
*/
void idMenuWidget::RemoveChild(std::shared_ptr<idMenuWidget> widget) {
	children.erase(std::remove(children.begin(), children.end(), widget), children.end());
	
	widget->SetParent(nullptr);
	widget = nullptr;
}

/*
========================
idMenuWidget::ReceiveEvent

Events received through this function are passed to the innermost focused widget first, and then
propagates back through each widget within the focus chain.  The first widget that handles the
event will stop propagation.

Each widget along the way will fire off an event to its observers, whether or not it actually
handles the event.

Note: How the focus chain is calculated:
Descend through GetFocus() calls until you reach a NULL focus.  The terminating widget is the
innermost widget, while *this* widget is the outermost widget.
========================
*/
void idMenuWidget::ReceiveEvent(const idWidgetEvent& event) {
	std::vector<std::shared_ptr<idMenuWidget>> focusChain;

	int focusRunawayCounter = focusChain.size();
	std::shared_ptr<idMenuWidget> focusedWidget = shared_from_this();
	while (focusedWidget && --focusRunawayCounter != 0) {
		focusChain.push_back(focusedWidget);
		focusedWidget = focusedWidget->GetFocus();
	}

	// If hitting this then more than likely you have a self-referential chain.  If that's not
	// the case, then you may need to increase the size of the focusChain list.
	//assert(focusRunawayCounter != 0);
	for (int focusIndex = focusChain.size() - 1; focusIndex >= 0; --focusIndex) {
		std::shared_ptr<idMenuWidget> const focusedWidget = focusChain[focusIndex];

		if (focusedWidget->ExecuteEvent(event)) {
			break;	// this widget has handled the event, so stop propagation
		}
	}
}

/*
========================
idMenuWidget::ExecuteEvent

Handles the event directly, and doesn't pass it through the focus chain.

This should only be used in very specific circumstances!  Most events should go to the focus.
========================
*/
bool idMenuWidget::ExecuteEvent(const idWidgetEvent& event) {
	std::vector<idWidgetAction>* const actions = GetEventActions(event.type);

	if (actions != nullptr) {
		for (size_t actionIndex = 0; actionIndex < actions->size(); ++actionIndex) {
			HandleAction((*actions)[actionIndex], event, shared_from_this());
		}
	}

	//SendEventToObservers(event);

	return actions != nullptr && actions->size() > 0;
}

/*
========================
idMenuWidget::ClearSprite
========================
*/
void idMenuWidget::ClearSprite() {
	if (!GetSprite()) {
		return;
	}
	GetSprite()->SetVisible(false);
	boundSprite = nullptr;
}

/*
========================
idMenuWidget::SetSpritePath
========================
*/
void idMenuWidget::SetSpritePath(const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5) {
	const char* args[] = { arg1, arg2, arg3, arg4, arg5 };
	const int numArgs = sizeof(args) / sizeof(args[0]);
	spritePath.clear();
	for (int i = 0; i < numArgs; ++i) {
		if (args[i] == NULL) {
			break;
		}
		spritePath.push_back(args[i]);
	}
}

/*
========================
idMenuWidget::SetSpritePath
========================
*/
void idMenuWidget::SetSpritePath(const std::vector<std::string>& spritePath_, const char* arg1, const char* arg2, const char* arg3, const char* arg4, const char* arg5) {
	const char* args[] = { arg1, arg2, arg3, arg4, arg5 };
	const int numArgs = sizeof(args) / sizeof(args[0]);
	spritePath = spritePath_;
	for (int i = 0; i < numArgs; ++i) {
		if (args[i] == NULL) {
			break;
		}
		spritePath.push_back(args[i]);
	}
}

/*
========================
idMenuWidget::GetSWFObject
========================
*/
std::shared_ptr<idSWF> idMenuWidget::GetSWFObject() {

	if (swfObj) {
		return swfObj;
	}

	if (parent.lock()) {
		return parent.lock()->GetSWFObject();
	}

	if (menuData) {
		return menuData->GetGUI();
	}

	return nullptr;
}

/*
========================
idMenuWidget::GetMenuData
========================
*/
std::shared_ptr<idMenuHandler> idMenuWidget::GetMenuData() {
	if (parent.lock()) {
		return parent.lock()->GetMenuData();
	}

	return menuData;
}

/*
========================
idMenuWidget::BindSprite

Takes the sprite path strings and resolves it to an actual sprite relative to a given root.

This is setup in this manner, because we can't resolve from path -> sprite immediately since
SWFs aren't necessarily loaded at the time widgets are instantiated.
========================
*/
bool idMenuWidget::BindSprite(std::shared_ptr<idSWFScriptObject> root) {
	const char* args[6] = { NULL };
	//assert(GetSpritePath().Num() > 0);
	for (size_t i = 0; i < GetSpritePath().size(); ++i) {
		args[i] = GetSpritePath()[i].c_str();
	}
	boundSprite = root->GetNestedSprite(args[0], args[1], args[2], args[3], args[4], args[5]);
	
	if (!boundSprite)
		common->Error("can't bind sprite: %s %s %s %s %s %s", args[0], args[1], args[2], args[3], args[4], args[5]);

	return boundSprite != nullptr;
}

/*
========================
idMenuWidget::SetFocusIndex
========================
*/
void idMenuWidget::SetFocusIndex(const size_t index, bool skipSound) {

	if (GetChildren().size() == 0) {
		return;
	}

	const size_t oldIndex = focusIndex;

	//assert(index >= 0 && index < GetChildren().size()); //&& oldIndex >= 0 && oldIndex < GetChildren().Num() );

	focusIndex = index;

	idSWFParmList parms;
	parms.Append(oldIndex);
	parms.Append(index);

	// need to mark the widget as having lost focus
	if (oldIndex != index && oldIndex >= 0 && oldIndex < GetChildren().size() && GetChildByIndex(oldIndex)->GetState() != widgetState_t::WIDGET_STATE_HIDDEN) {
		GetChildByIndex(oldIndex)->ReceiveEvent(idWidgetEvent(widgetEvent_t::WIDGET_EVENT_FOCUS_OFF, 0, nullptr, parms));
	}

	//assert( GetChildByIndex( index ).GetState() != WIDGET_STATE_HIDDEN );
	GetChildByIndex(index)->ReceiveEvent(idWidgetEvent(widgetEvent_t::WIDGET_EVENT_FOCUS_ON, 0, nullptr, parms));
}

/*
========================
idMenuWidget_Button::SetState

Transitioning from the current button state to the new button state
========================
*/
void idMenuWidget::SetState(const widgetState_t state) {
	if (GetSprite()) {
		// FIXME: will need some more intelligence in the transitions to go from, say,
		// selected_up -> up ... but this should work fine for now.
		if (state == widgetState_t::WIDGET_STATE_HIDDEN) {
			GetSprite()->SetVisible(false);
		}
		else {
			GetSprite()->SetVisible(true);
			if (state == widgetState_t::WIDGET_STATE_DISABLED) {
				//GetSprite()->PlayFrame("disabled");
			}
			else if (state == widgetState_t::WIDGET_STATE_SELECTING) {
				if (widgetState == widgetState_t::WIDGET_STATE_NORMAL) {
					GetSprite()->SetColor(Screen::ConsoleColor::Yellow); // transition from unselected to selected
				}
				else {
					GetSprite()->SetColor(Screen::ConsoleColor::Yellow);
				}
			}
			else if (state == widgetState_t::WIDGET_STATE_SELECTED) {
				GetSprite()->SetColor(Screen::ConsoleColor::Yellow);
			}
			else if (state == widgetState_t::WIDGET_STATE_NORMAL) {
				if (widgetState == widgetState_t::WIDGET_STATE_SELECTING) {
					//GetSprite()->PlayFrame("unselecting");	// transition from selected to unselected
				}
				else if (widgetState != widgetState_t::WIDGET_STATE_HIDDEN && widgetState != widgetState_t::WIDGET_STATE_NORMAL) {
					//GetSprite()->PlayFrame("out");
				}
				else {
					//GetSprite()->PlayFrame("up");
				}
				GetSprite()->SetColor(Screen::ConsoleColor::White);
			}
		}

		Update();
	}

	widgetState = state;
}

/*
========================
idMenuWidget::HandleAction
========================
*/
bool idMenuWidget::HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled) {

	bool handled = false;
	if (GetParent()) {
		handled = GetParent()->HandleAction(action, event, widget);
	}
	else {

		if (forceHandled) {
			return false;
		}

		std::shared_ptr<idMenuHandler> data = GetMenuData();
		if (data) {
			return data->HandleAction(action, event, widget, false);
		}
	}

	return handled;
}

/*
========================
idMenuWidget::GetEventActions
========================
*/
std::vector<idWidgetAction>* idMenuWidget::GetEventActions(const widgetEvent_t eventType) {
	if (eventActionLookup[static_cast<int>(eventType)] == INVALID_ACTION_INDEX) {
		return nullptr;
	}
	return &eventActions[eventActionLookup[static_cast<int>(eventType)]];
}

/*
========================
idMenuWidget::AddEventAction
========================
*/
idWidgetAction& idMenuWidget::AddEventAction(const widgetEvent_t eventType) {
	if (eventActionLookup[static_cast<int>(eventType)] == INVALID_ACTION_INDEX) {
		eventActionLookup[static_cast<int>(eventType)] = eventActions.size();
		eventActions.emplace_back();
	}
	eventActions[eventActionLookup[static_cast<int>(eventType)]].emplace_back();

	return eventActions[eventActionLookup[static_cast<int>(eventType)]].back();
}

/*
========================
idMenuWidget::ClearEventActions
========================
*/
void idMenuWidget::ClearEventActions() {
	eventActions.clear();
	eventActionLookup.clear();
	eventActionLookup.resize(static_cast<int>(widgetEvent_t::MAX_WIDGET_EVENT));
	for (size_t i = 0; i < eventActionLookup.size(); ++i) {
		eventActionLookup[i] = INVALID_ACTION_INDEX;
	}
}
