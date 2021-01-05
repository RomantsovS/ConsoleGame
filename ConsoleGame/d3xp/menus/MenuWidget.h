#ifndef D3XP_MENUS_MENUWIDGET_H__
#define D3XP_MENUS_MENUWIDGET_H__

#include "MenuHandler.h"

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
		/*parms = src.parms;
		scriptFunction = src.scriptFunction;
		if (scriptFunction != NULL) {
			scriptFunction->AddRef();
		}*/
	}

	~idWidgetAction() {
		/*if (scriptFunction != NULL) {
			scriptFunction->Release();
		}*/
	}

	void Set(widgetAction_t action_) {
		action = action_;
		//parms.Clear();
	}

	//idSWFScriptFunction* GetScriptFunction() { return scriptFunction; }
	const widgetAction_t GetType() const { return action; }

private:
	widgetAction_t action;
	//idSWFScriptFunction* scriptFunction;
};

class idMenuHandler;
class idMenuWidget;
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
	idMenuWidget();

	virtual ~idMenuWidget();

	void Cleanup();

	// typically this is where the allocations for a widget will occur: sub widgets, etc.
	// Note that SWF sprite objects may not be accessible at this point.
	virtual void Initialize(std::shared_ptr<idMenuHandler> data) { menuData = data; }

	virtual void Update() {}

	std::shared_ptr<GUI> GetGUIObject();

	// actually binds the sprite. this must be called after setting sprite path!
	std::shared_ptr<GUISpriteInstance> GetSprite() { return boundSprite; }
	bool BindSprite(std::shared_ptr<GUIScriptObject> root);

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

protected:
	std::shared_ptr<idMenuHandler> menuData;
	std::shared_ptr<GUI> guiObj;
	std::shared_ptr<GUISpriteInstance> boundSprite;
	std::weak_ptr<idMenuWidget> parent;
	idMenuWidgetList children;
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

	virtual void Update() override;

	//---------------
	// Model
	//---------------
	void SetLabel(const std::string& label) { btnLabel = label; }
	const std::string& GetLabel() const { return btnLabel; }

protected:
	std::string btnLabel;
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

	virtual void Update();
	//virtual bool ExecuteEvent(const idWidgetEvent& event);

	buttonInfo_t* GetButton(const button_t button) { return &buttons[button]; }
	void ClearAllButtons();

private:
	std::vector<buttonInfo_t> buttons;
};

#endif
