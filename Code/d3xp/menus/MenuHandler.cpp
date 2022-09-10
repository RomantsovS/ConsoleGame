#include "idlib/precompiled.h"


#include "../Game_local.h"

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::idMenuHandler() :
	activeScreen(-1),
	nextScreen(-1),
	gui(nullptr),
	cmdBar(nullptr) {
#ifdef DEBUG_PRINT_Ctor_Dtor
	common->DPrintf("%s ctor\n", "idMenuHandler");
#endif // DEBUG_PRINT_Ctor_Dtor
	menuScreens.resize(MAX_SCREEN_AREAS);
}

/*
================================================
idMenuHandler::~idMenuHandler
================================================
*/
idMenuHandler::~idMenuHandler() {
#ifdef DEBUG_PRINT_Ctor_Dtor
	if(isCommonExists)
		common->DPrintf("%s dtor\n", "idMenuHandler");
#endif // DEBUG_PRINT_Ctor_Dtor
	Cleanup();
}

/*
================================================
idMenuHandler::Initialize
================================================
*/
void idMenuHandler::Initialize(const std::string& filename) {
	Cleanup();

	gui = std::make_shared<idSWF>(filename);

	gui->Init();
}

/*
================================================
idMenuHandler::AddChild
================================================
*/
void idMenuHandler::AddChild(std::shared_ptr<idMenuWidget> widget) {
	widget->SetSWFObj(gui);
	//widget->SetHandlerIsParent(true);
	children.push_back(widget);
}

/*
================================================
idMenuHandler::Cleanup
================================================
*/
void idMenuHandler::Cleanup() noexcept {
	for (size_t index = 0; index < children.size(); ++index) {
		children[index] = nullptr;
	}
	children.clear();

	for (int index = 0; index < MAX_SCREEN_AREAS; ++index) {
		if (menuScreens[index]) {
			menuScreens[index] = nullptr;
		}
	}

	gui = nullptr;
}

/*
================================================
idMenuHandler::IsActive
================================================
*/
bool idMenuHandler::IsActive() noexcept {
	if (!gui) {
		return false;
	}

	return gui->IsActive();
}

/*
================================================
idMenuHandler::ActivateMenu
================================================
*/
void idMenuHandler::ActivateMenu(bool show) noexcept {

	if (!gui)
		return;

	if (!show) {
		gui->Activate(show);
		return;
	}

	gui->Activate(show);
}

/*
================================================
idMenuHandler::Update
================================================
*/
void idMenuHandler::Update() {
	PumpWidgetActionRepeater();

	if (gui && gui->IsActive()) {
		gui->Render(renderSystem, Sys_Milliseconds());
	}
}

/*
================================================
idMenuHandler::UpdateChildren
================================================
*/
void idMenuHandler::UpdateChildren() noexcept {
	for (size_t index = 0; index < children.size(); ++index) {
		if (children[index]) {
			children[index]->Update();
		}
	}
}

/*
================================================
idMenuHandler::HandleSWFEvent
================================================
*/
bool idMenuHandler::HandleGuiEvent(const sysEvent_t* sev) {

	if (gui && activeScreen != -1) {
		return gui->HandleEvent(sev);
	}

	return false;
}

/*
================================================
idMenuHandler::Update
================================================
*/
bool idMenuHandler::HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled) {

	const widgetAction_t actionType = action.GetType();
	const idSWFParmList& parms = action.GetParms();

	switch (actionType) {
	/*case widgetAction_t::WIDGET_ACTION_ADJUST_FIELD: {
		if (widget && widget->GetDataSource() != NULL) {
			widget->GetDataSource()->AdjustField(widget->GetDataSourceFieldIndex(), parms[0].ToInteger());
			widget->Update();
		}
		return true;
	}*/
	case widgetAction_t::WIDGET_ACTION_FUNCTION: {
		if (action.GetScriptFunction()) {
			action.GetScriptFunction()->Call(event.thisObject, event.parms);
		}
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_PRESS_FOCUSED: {
		std::shared_ptr<idMenuScreen> const screen = menuScreens[activeScreen];
		if (screen) {
			idWidgetEvent pressEvent(widgetEvent_t::WIDGET_EVENT_PRESS, 0, event.thisObject, idSWFParmList());
			screen->ReceiveEvent(pressEvent);
		}
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_START_REPEATER: {
		idWidgetAction repeatAction;
		widgetAction_t repeatActionType = static_cast<widgetAction_t>(parms[0]->ToInteger());
		idassert(parms.size() >= 2);
		int repeatDelay = DEFAULT_REPEAT_TIME;
		if (parms.size() >= 3) {
			repeatDelay = parms[2]->ToInteger();
		}
		repeatAction.Set(repeatActionType, *parms[1], repeatDelay);
		StartWidgetActionRepeater(widget, repeatAction, event);
		return true;
	}
	case widgetAction_t::WIDGET_ACTION_STOP_REPEATER: {
		ClearWidgetActionRepeater();
		return true;
	}
	}

	/*if (!widget->GetHandlerIsParent()) {
		for (int index = 0; index < children.size(); ++index) {
			if (children[index] != NULL) {
				if (children[index]->HandleAction(action, event, widget, forceHandled)) {
					return true;
				}
			}
		}
	}*/

	return false;
}

/*
========================
idMenuHandler::StartWidgetActionRepeater
========================
*/
void idMenuHandler::StartWidgetActionRepeater(idMenuWidget* widget, const idWidgetAction& action, const idWidgetEvent& event) {
	if (actionRepeater.isActive && actionRepeater.action == action) {
		return;	// don't attempt to reactivate an already active repeater
	}

	actionRepeater.isActive = true;
	actionRepeater.action = action;
	actionRepeater.widget = widget;
	actionRepeater.event = event;
	actionRepeater.numRepetitions = 0;
	actionRepeater.nextRepeatTime = 0;
	actionRepeater.screenIndex = activeScreen;	// repeaters are cleared between screens

	if (action.GetParms().size() == 2) {
		actionRepeater.repeatDelay = action.GetParms()[1]->ToInteger();
	}
	else {
		actionRepeater.repeatDelay = DEFAULT_REPEAT_TIME;
	}

	// do the first event immediately
	PumpWidgetActionRepeater();
}

/*
========================
idMenuHandler::PumpWidgetActionRepeater
========================
*/
void idMenuHandler::PumpWidgetActionRepeater() {
	if (!actionRepeater.isActive) {
		return;
	}

	if (activeScreen != actionRepeater.screenIndex || nextScreen != activeScreen) { // || common->IsDialogActive() ) {
		actionRepeater.isActive = false;
		return;
	}

	if (actionRepeater.nextRepeatTime > Sys_Milliseconds()) {
		return;
	}

	// need to hold down longer on the first iteration before we continue to scroll
	if (actionRepeater.numRepetitions == 0) {
		actionRepeater.nextRepeatTime = Sys_Milliseconds() + 400;
	}
	else {
		actionRepeater.nextRepeatTime = Sys_Milliseconds() + actionRepeater.repeatDelay;
	}

	if (true/*verify(actionRepeater.widget != NULL)*/) {
		actionRepeater.widget->HandleAction(actionRepeater.action, actionRepeater.event, actionRepeater.widget);
		actionRepeater.numRepetitions++;
	}
}

/*
========================
idMenuHandler::ClearWidgetActionRepeater
========================
*/
void idMenuHandler::ClearWidgetActionRepeater() noexcept {
	actionRepeater.isActive = false;
}
