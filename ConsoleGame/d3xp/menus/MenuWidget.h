#ifndef D3XP_MENUS_MENUWIDGET_H__
#define D3XP_MENUS_MENUWIDGET_H__

class idMenuHandler;
class idMenuWidget;

enum class widgetEvent_t {
	WIDGET_EVENT_PRESS,
	WIDGET_EVENT_RELEASE,
	WIDGET_EVENT_ROLL_OVER,
	WIDGET_EVENT_ROLL_OUT,
	WIDGET_EVENT_FOCUS_ON,
	WIDGET_EVENT_FOCUS_OFF,

	WIDGET_EVENT_SCROLL_UP_LSTICK,
	WIDGET_EVENT_SCROLL_UP_LSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_DOWN_LSTICK,
	WIDGET_EVENT_SCROLL_DOWN_LSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_LEFT_LSTICK,
	WIDGET_EVENT_SCROLL_LEFT_LSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_RIGHT_LSTICK,
	WIDGET_EVENT_SCROLL_RIGHT_LSTICK_RELEASE,

	WIDGET_EVENT_SCROLL_UP_RSTICK,
	WIDGET_EVENT_SCROLL_UP_RSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_DOWN_RSTICK,
	WIDGET_EVENT_SCROLL_DOWN_RSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_LEFT_RSTICK,
	WIDGET_EVENT_SCROLL_LEFT_RSTICK_RELEASE,
	WIDGET_EVENT_SCROLL_RIGHT_RSTICK,
	WIDGET_EVENT_SCROLL_RIGHT_RSTICK_RELEASE,

	WIDGET_EVENT_SCROLL_UP,
	WIDGET_EVENT_SCROLL_UP_RELEASE,
	WIDGET_EVENT_SCROLL_DOWN,
	WIDGET_EVENT_SCROLL_DOWN_RELEASE,
	WIDGET_EVENT_SCROLL_LEFT,
	WIDGET_EVENT_SCROLL_LEFT_RELEASE,
	WIDGET_EVENT_SCROLL_RIGHT,
	WIDGET_EVENT_SCROLL_RIGHT_RELEASE,

	WIDGET_EVENT_DRAG_START,
	WIDGET_EVENT_DRAG_STOP,

	WIDGET_EVENT_SCROLL_PAGEDWN,
	WIDGET_EVENT_SCROLL_PAGEDWN_RELEASE,
	WIDGET_EVENT_SCROLL_PAGEUP,
	WIDGET_EVENT_SCROLL_PAGEUP_RELEASE,

	WIDGET_EVENT_SCROLL,
	WIDGET_EVENT_SCROLL_RELEASE,
	WIDGET_EVENT_BACK,
	WIDGET_EVENT_COMMAND,
	WIDGET_EVENT_TAB_NEXT,
	WIDGET_EVENT_TAB_PREV,
	MAX_WIDGET_EVENT
};

enum class scrollType_t {
	SCROLL_SINGLE,		// scroll a single unit
	SCROLL_PAGE,		// scroll a page
	SCROLL_FULL,		// scroll all the way to the end
	SCROLL_TOP,			// scroll to the first selection
	SCROLL_END,			// scroll to the last selection
};

enum class widgetAction_t{
	WIDGET_ACTION_NONE,
	WIDGET_ACTION_COMMAND,
	WIDGET_ACTION_FUNCTION,					// call the SWF function
	WIDGET_ACTION_SCROLL_VERTICAL,			// scroll something. takes one param = amount to scroll (can be negative)
	WIDGET_ACTION_SCROLL_VERTICAL_VARIABLE,
	WIDGET_ACTION_SCROLL_PAGE,
	WIDGET_ACTION_SCROLL_HORIZONTAL,		// scroll something. takes one param = amount to scroll (can be negative)
	WIDGET_ACTION_SCROLL_TAB,
	WIDGET_ACTION_START_REPEATER,
	WIDGET_ACTION_STOP_REPEATER,
	WIDGET_ACTION_ADJUST_FIELD,
	WIDGET_ACTION_PRESS_FOCUSED,
	WIDGET_ACTION_JOY3_ON_PRESS,
	WIDGET_ACTION_JOY4_ON_PRESS,
	//
	WIDGET_ACTION_GOTO_MENU,
	WIDGET_ACTION_GO_BACK,
	WIDGET_ACTION_EXIT_GAME,
	WIDGET_ACTION_LAUNCH_MULTIPLAYER,
	WIDGET_ACTION_MENU_BAR_SELECT,
	WIDGET_ACTION_EMAIL_HOVER,
	// PDA USER DATA ACTIONS
	WIDGET_ACTION_PDA_SELECT_USER,
	WIDGET_ACTION_SELECT_GAMERTAG,
	WIDGET_ACTION_PDA_SELECT_NAV,
	WIDGET_ACTION_SELECT_PDA_AUDIO,
	WIDGET_ACTION_SELECT_PDA_VIDEO,
	WIDGET_ACTION_SELECT_PDA_ITEM,
	WIDGET_ACTION_SCROLL_DRAG,
	// PDA EMAIL ACTIONS
	WIDGET_ACTION_PDA_SELECT_EMAIL,
	WIDGET_ACTION_PDA_CLOSE,
	WIDGET_ACTION_REFRESH,
	WIDGET_ACTION_MUTE_PLAYER,
	MAX_WIDGET_ACTION
};

enum class actionHandler_t {
	WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER,
	WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER_VARIABLE,
	WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER,
	WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER_VARIABLE,
	WIDGET_ACTION_EVENT_SCROLL_LEFT_START_REPEATER,
	WIDGET_ACTION_EVENT_SCROLL_RIGHT_START_REPEATER,
	WIDGET_ACTION_EVENT_SCROLL_PAGE_DOWN_START_REPEATER,
	WIDGET_ACTION_EVENT_SCROLL_PAGE_UP_START_REPEATER,
	WIDGET_ACTION_EVENT_STOP_REPEATER,
	WIDGET_ACTION_EVENT_TAB_NEXT,
	WIDGET_ACTION_EVENT_TAB_PREV,
	WIDGET_ACTION_EVENT_DRAG_START,
	WIDGET_ACTION_EVENT_DRAG_STOP,
	WIDGET_ACTION_EVENT_JOY3_ON_PRESS,
};

/*
================================================
idWidgetEvent
================================================
*/
class idWidgetEvent {
public:
	idWidgetEvent() :
		type(widgetEvent_t::WIDGET_EVENT_PRESS),
		arg(0),
		thisObject(nullptr) {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "idWidgetEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	idWidgetEvent(const widgetEvent_t type_, const int arg_, std::shared_ptr<idSWFScriptObject> thisObject_, const idSWFParmList& parms_) :
		type(type_),
		arg(arg_),
		thisObject(thisObject_),
		parms(parms_) {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "idWidgetEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	widgetEvent_t type;
	size_t arg;
	std::shared_ptr<idSWFScriptObject> thisObject;
	idSWFParmList parms;
};

/*
================================================
idWidgetAction
================================================
*/
class idWidgetAction {
public:
	idWidgetAction() :
		action(widgetAction_t::WIDGET_ACTION_NONE)//,
		//scriptFunction(NULL)
	{
	}

	idWidgetAction(const idWidgetAction& src) {
		action = src.action;
		parms = src.parms;
		/*scriptFunction = src.scriptFunction;
		if (scriptFunction != NULL) {
			scriptFunction->AddRef();
		}*/
	}

	~idWidgetAction() {
		/*if (scriptFunction != NULL) {
			scriptFunction->Release();
		}*/
	}

	void operator=(const idWidgetAction& src) {
		action = src.action;
		parms = src.parms;
		scriptFunction = src.scriptFunction;
	}

	bool operator==(const idWidgetAction& otherAction) const {
		if (GetType() != otherAction.GetType()
			|| GetParms().size() != otherAction.GetParms().size()) {
			return false;
		}

		// everything else is equal, so check all parms. NOTE: this assumes we are only sending
		// integral types.
		for (size_t i = 0; i < GetParms().size(); ++i) {
			if (GetParms()[i]->GetType() != otherAction.GetParms()[i]->GetType()
				|| GetParms()[i]->ToInteger() != otherAction.GetParms()[i]->ToInteger()) {
				return false;
			}
		}

		return true;
	}

	void Set(std::shared_ptr<idSWFScriptFunction> function) {
		action = widgetAction_t::WIDGET_ACTION_FUNCTION;
		scriptFunction = function;
	}

	void Set(widgetAction_t action_) {
		action = action_;
		parms.clear();
	}

	void Set(widgetAction_t action_, const idSWFScriptVar& var1) {
		action = action_;
		parms.clear();
		parms.Append(var1);
	}

	void Set(widgetAction_t action_, const idSWFScriptVar& var1, const idSWFScriptVar& var2) {
		action = action_;
		parms.clear();
		parms.Append(var1);
		parms.Append(var2);
	}

	std::shared_ptr<idSWFScriptFunction> GetScriptFunction() { return scriptFunction; }
	const widgetAction_t GetType() const { return action; }
	const idSWFParmList& GetParms() const { return parms; }

private:
	widgetAction_t action;
	idSWFParmList parms;
	std::shared_ptr<idSWFScriptFunction> scriptFunction;
};

using idMenuWidgetList = std::vector<std::shared_ptr<idMenuWidget>>;

/*
================================================
idMenuWidget

We're using a model/view architecture, so this is the combination of both model and view.  The
other part of the view is the SWF itself.
================================================
*/
class idMenuWidget : public std::enable_shared_from_this<idMenuWidget> {
public:
	/*
	================================================
	WrapWidgetSWFEvent
	================================================
	*/
	class WrapWidgetSWFEvent : public idSWFScriptFunction {
	public:
		WrapWidgetSWFEvent(std::shared_ptr<idMenuWidget> widget, const widgetEvent_t event, const int eventArg) :
			targetWidget(widget),
			targetEvent(event),
			targetEventArg(eventArg) {
#ifdef DEBUG_PRINT_Ctor_Dtor
			common->DPrintf("%s ctor\n", "WrapWidgetSWFEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
		}
		~WrapWidgetSWFEvent() {
#ifdef DEBUG_PRINT_Ctor_Dtor
			common->DPrintf("%s dtor\n", "WrapWidgetSWFEvent");
#endif // DEBUG_PRINT_Ctor_Dtor
		}

		idSWFScriptVar Call(std::shared_ptr<idSWFScriptObject> thisObject, const idSWFParmList& parms) override {
			targetWidget.lock()->ReceiveEvent(idWidgetEvent(targetEvent, targetEventArg, thisObject, parms));
			return idSWFScriptVar();
		}

	private:
		std::weak_ptr<idMenuWidget> targetWidget;
		widgetEvent_t	targetEvent;
		int				targetEventArg;
	};

	enum class widgetState_t {
		WIDGET_STATE_HIDDEN,	// hidden
		WIDGET_STATE_NORMAL,	// normal
		WIDGET_STATE_SELECTING,	// going into the selected state
		WIDGET_STATE_SELECTED,	// fully selected
		WIDGET_STATE_DISABLED,	// disabled
		WIDGET_STATE_MAX
	};

	idMenuWidget();

	virtual ~idMenuWidget();

	void Cleanup();

	// typically this is where the allocations for a widget will occur: sub widgets, etc.
	// Note that SWF sprite objects may not be accessible at this point.
	virtual void Initialize(std::shared_ptr<idMenuHandler> data) { menuData = data; }

	virtual void Update() {}

	widgetState_t GetState() const { return widgetState; }
	void SetState(const widgetState_t state);

	std::shared_ptr<idSWF> GetSWFObject();
	std::shared_ptr<idMenuHandler> GetMenuData();

	// actually binds the sprite. this must be called after setting sprite path!
	std::shared_ptr<idSWFSpriteInstance> GetSprite() { return boundSprite; }
	bool BindSprite(std::shared_ptr<idSWFScriptObject> root);
	void ClearSprite();

	void SetSpritePath(const char* arg1, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL);
	void SetSpritePath(const std::vector<std::string>& spritePath_, const char* arg1 = NULL, const char* arg2 = NULL, const char* arg3 = NULL, const char* arg4 = NULL, const char* arg5 = NULL);
	std::vector<std::string>& GetSpritePath() { return spritePath; }

	//------------------------
	// Event Handling
	//------------------------
	virtual bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false);
	void ReceiveEvent(const idWidgetEvent& event);

	// Executes an event in the context of this widget.  Only rarely should this be called
	// directly.  Instead calls should go through ReceiveEvent which will propagate the event
	// through the standard focus order.
	virtual bool ExecuteEvent(const idWidgetEvent& event);

	// returns the list of actions for a given event, or NULL if no actions are registered for
	// that event.  Events should not be directly added to the returned list.  Instead use
	// AddEventAction for adding new events.
	std::vector<idWidgetAction>* GetEventActions(const widgetEvent_t eventType);

	// allocates an action for the given event
	idWidgetAction& AddEventAction(const widgetEvent_t eventType);
	void ClearEventActions();

	//------------------------
	// Data modeling
	//------------------------
	/*void								SetDataSource(idMenuDataSource* dataSource, const int fieldIndex);
	idMenuDataSource* GetDataSource() { return dataSource; }
	void								SetDataSourceFieldIndex(const int dataSourceFieldIndex_) { dataSourceFieldIndex = dataSourceFieldIndex_; }
	int									GetDataSourceFieldIndex() const { return dataSourceFieldIndex; }
	*/
	std::shared_ptr<idMenuWidget> GetFocus() { return (focusIndex >= 0 && focusIndex < children.size()) ? children[focusIndex] : nullptr; }
	size_t GetFocusIndex() const { return focusIndex; }
	void SetFocusIndex(const size_t index, bool skipSound = false);


	//------------------------
	// Hierarchy
	//------------------------
	idMenuWidgetList& GetChildren() { return children; }
	const idMenuWidgetList& GetChildren() const { return children; }
	std::shared_ptr<idMenuWidget> GetChildByIndex(const int index) const { return children[index]; }

	void AddChild(std::shared_ptr<idMenuWidget> widget);
	void RemoveChild(std::shared_ptr<idMenuWidget> widget);

	std::shared_ptr<idMenuWidget> GetParent() { return parent.lock(); }
	const std::shared_ptr<idMenuWidget> GetParent() const { return parent.lock(); }
	void SetParent(std::shared_ptr<idMenuWidget> parent_) { parent = parent_; }
	void SetSWFObj(std::shared_ptr<idSWF> obj) { swfObj = obj; }
protected:
	std::weak_ptr<idMenuHandler> menuData;
	std::weak_ptr<idSWF> swfObj;
	std::shared_ptr<idSWFSpriteInstance> boundSprite;
	std::weak_ptr<idMenuWidget> parent;
	std::vector<std::string> spritePath;
	idMenuWidgetList children;

	static const int INVALID_ACTION_INDEX = -1;
	std::vector<std::vector<idWidgetAction>> eventActions;
	std::vector<int> eventActionLookup;

	widgetState_t widgetState;
	size_t focusIndex;
};

/*
================================================
idMenuWidget_Button

This might be better named "ComboButton", as it contains behavior for several controls along
with standard button behavior.
================================================
*/
class idMenuWidget_Button : public idMenuWidget {
public:
	idMenuWidget_Button() {
	}

	virtual ~idMenuWidget_Button() {}
	virtual bool ExecuteEvent(const idWidgetEvent& event);
	virtual void Update() override;

	//---------------
	// Model
	//---------------
	void SetLabel(const std::string& label) { btnLabel = label; }
	const std::string& GetLabel() const { return btnLabel; }
	void SetValues(std::vector<std::string>& list);
protected:
	std::vector<std::string> values;
	std::string btnLabel;
};

/*
================================================
idMenuWidget_NavButton
================================================
*/
class idMenuWidget_MenuButton : public idMenuWidget_Button {
public:

	idMenuWidget_MenuButton() :
		xPos(0) {
	}

	virtual void Update();
	void SetPosition(float pos) { xPos = pos; }

private:

	float xPos;

};

/*
================================================
idMenuWidget_List

Stores a list of widgets but displays only a segment of them at a time.
================================================
*/
class idMenuWidget_List : public idMenuWidget {
public:
	idMenuWidget_List() :
		numVisibleOptions(0),
		viewOffset(0),
		viewIndex(0),
		allowWrapping(false) {

	}

	virtual void				Update();
	virtual bool				HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false);
	//virtual void				ObserveEvent(const idMenuWidget& widget, const idWidgetEvent& event);
	virtual void				Scroll(const int scrollIndexAmount, const bool wrapAround = false);
	virtual void				ScrollOffset(const int scrollIndexAmount);
	virtual size_t				GetTotalNumberOfOptions() const { return GetChildren().size(); }
	//virtual bool				PrepareListElement(idMenuWidget& widget, const int childIndex) { return true; }

	bool						IsWrappingAllowed() const { return allowWrapping; }
	void						SetWrappingAllowed(const bool allow) { allowWrapping = allow; }

	void						SetNumVisibleOptions(const size_t numVisibleOptions_) { numVisibleOptions = numVisibleOptions_; }
	size_t						GetNumVisibleOptions() const { return numVisibleOptions; }

	int							GetViewOffset() const { return viewOffset; }
	void						SetViewOffset(const int offset) { viewOffset = offset; }

	int							GetViewIndex() const { return viewIndex; }
	void						SetViewIndex(const int index) { viewIndex = index; }

	void						CalculatePositionFromIndexDelta(int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int indexDelta, const bool allowWrapping, const bool wrapAround = false) const;
	void						CalculatePositionFromOffsetDelta(int& outIndex, int& outOffset, const int currentIndex, const int currentOffset, const int windowSize, const int maxSize, const int offsetDelta) const;

private:
	size_t						numVisibleOptions;
	int							viewOffset;
	int							viewIndex;
	bool						allowWrapping;
};

/*
================================================
idMenuWidget_CommandBar
================================================
*/
class idMenuWidget_CommandBar : public idMenuWidget {
public:
	enum button_t {
		BUTTON_JOY1,
		BUTTON_JOY2,
		BUTTON_JOY3,
		BUTTON_JOY4,
		BUTTON_JOY10,
		BUTTON_TAB,
		MAX_BUTTONS
	};

	struct buttonInfo_t {
		std::string label;			// empty labels are treated as hidden buttons
		idWidgetAction action;
	};

	idMenuWidget_CommandBar() {
		buttons.resize(MAX_BUTTONS);
	}

	virtual void Update() override;
	virtual bool ExecuteEvent(const idWidgetEvent& event);

	buttonInfo_t* GetButton(const button_t button) { return &buttons[button]; }
	void ClearAllButtons();

private:
	std::vector<buttonInfo_t> buttons;
};

/*
================================================
idMenuWidget_DynamicList
================================================
*/
class idMenuWidget_DynamicList : public idMenuWidget_List {
public:

	idMenuWidget_DynamicList() :
		controlList(false),
		ignoreColor(false) {
	}

	virtual void				Update();
	virtual void				Initialize(std::shared_ptr<idMenuHandler> data);
	virtual size_t				GetTotalNumberOfOptions() const;
	virtual bool				PrepareListElement(std::shared_ptr<idMenuWidget>, const size_t childIndex);

	//virtual void				Recalculate();
	virtual void				SetListData(std::vector<std::vector<std::string>>& list);

	void						SetControlList(bool val) { controlList = val; }
	void						SetIgnoreColor(bool val) { ignoreColor = val; }

protected:
	std::vector<std::vector<std::string>> listItemInfo;
	bool						controlList;
	bool						ignoreColor;
};

/*
================================================
idMenuWidget_NavBar

The nav bar is set up with the main option being at the safe frame line.
================================================
*/
class idMenuWidget_MenuBar : public idMenuWidget_DynamicList {
public:

	idMenuWidget_MenuBar() :
		totalWidth(0.0f),
		buttonPos(0.0f),
		rightSpacer(0.0f) {
	}

	virtual void				Update();
	virtual void				Initialize(std::shared_ptr<idMenuHandler> data);
	virtual void				SetButtonSpacing(float rSpace) { rightSpacer = rSpace; }
	virtual bool				PrepareListElement(std::shared_ptr<idMenuWidget>, const size_t childIndex) override;
	virtual void				SetListHeadings(std::vector<std::string>& list);
	virtual size_t				GetTotalNumberOfOptions() const override;

private:

	std::vector<std::string> headings;
	float						totalWidth;
	float						buttonPos;
	float						rightSpacer;

};

/*
================================================
idWidgetActionHandler
================================================
*/
class idWidgetActionHandler : public idSWFScriptFunction {
public:
	idWidgetActionHandler(std::shared_ptr<idMenuWidget> widget, actionHandler_t actionEventType, widgetEvent_t _event) :
		targetWidget(widget),
		type(actionEventType),
		targetEvent(_event) {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "idWidgetActionHandler");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	virtual ~idWidgetActionHandler() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s dtor\n", "idWidgetActionHandler");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	idSWFScriptVar Call(std::shared_ptr<idSWFScriptObject> thisObject, const idSWFParmList& parms) {

		idWidgetAction action;
		bool handled = false;
		switch (type) {
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_VERTICAL), 1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_VERTICAL), -1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_DOWN_START_REPEATER_VARIABLE: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_VERTICAL_VARIABLE), 1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_UP_START_REPEATER_VARIABLE: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_VERTICAL_VARIABLE), -1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_PAGE_DOWN_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_PAGE), 1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_PAGE_UP_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_PAGE), -1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_STOP_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_STOP_REPEATER);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_TAB_NEXT: {
			action.Set(widgetAction_t::WIDGET_ACTION_SCROLL_TAB, 1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_TAB_PREV: {
			action.Set(widgetAction_t::WIDGET_ACTION_SCROLL_TAB, -1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_JOY3_ON_PRESS: {
			action.Set(widgetAction_t::WIDGET_ACTION_JOY3_ON_PRESS);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_LEFT_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_HORIZONTAL), -1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_SCROLL_RIGHT_START_REPEATER: {
			action.Set(widgetAction_t::WIDGET_ACTION_START_REPEATER, static_cast<int>(widgetAction_t::WIDGET_ACTION_SCROLL_HORIZONTAL), 1);
			handled = true;
			break;
		}
		case actionHandler_t::WIDGET_ACTION_EVENT_DRAG_START: {
			action.Set(widgetAction_t::WIDGET_ACTION_SCROLL_DRAG);
			handled = true;
			break;
		}
		}

		if (handled) {
			targetWidget.lock()->HandleAction(action, idWidgetEvent(targetEvent, 0, thisObject, parms), targetWidget.lock());
		}

		return idSWFScriptVar();
	}

private:
	std::weak_ptr<idMenuWidget> targetWidget;
	actionHandler_t type;
	widgetEvent_t targetEvent;
};

#endif
