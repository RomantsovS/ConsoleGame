#ifndef D3XP_MENUS_MENUHANDLER_H__
#define D3XP_MENUS_MENUHANDLER_H__

#include <string>
#include <vector>
#include <memory>

#include "../../sys/sys_public.h"
#include "../../gui/GUI.h"

class idMenuWidget;
class idMenuWidget_CommandBar;

enum class shellAreas_t {
	SHELL_AREA_INVALID = -1,
	SHELL_AREA_START,
	SHELL_AREA_ROOT,
	SHELL_AREA_DEV,
	SHELL_AREA_CAMPAIGN,
	SHELL_AREA_LOAD,
	SHELL_AREA_SAVE,
	SHELL_AREA_NEW_GAME,
	SHELL_AREA_GAME_OPTIONS,
	SHELL_AREA_SYSTEM_OPTIONS,
	SHELL_AREA_MULTIPLAYER,
	SHELL_AREA_GAME_LOBBY,
	SHELL_AREA_STEREOSCOPICS,
	SHELL_AREA_PARTY_LOBBY,
	SHELL_AREA_SETTINGS,
	SHELL_AREA_AUDIO,
	SHELL_AREA_VIDEO,
	SHELL_AREA_KEYBOARD,
	SHELL_AREA_CONTROLS,
	SHELL_AREA_CONTROLLER_LAYOUT,
	SHELL_AREA_GAMEPAD,
	SHELL_AREA_PAUSE,
	SHELL_AREA_LEADERBOARDS,
	SHELL_AREA_PLAYSTATION,
	SHELL_AREA_DIFFICULTY,
	SHELL_AREA_RESOLUTION,
	SHELL_AREA_MATCH_SETTINGS,
	SHELL_AREA_MODE_SELECT,
	SHELL_AREA_BROWSER,
	SHELL_AREA_CREDITS,
	SHELL_NUM_AREAS
};

enum class shellState_t {
	SHELL_STATE_INVALID = -1,
	SHELL_STATE_PRESS_START,
	SHELL_STATE_IDLE,
	SHELL_STATE_PARTY_LOBBY,
	SHELL_STATE_GAME_LOBBY,
	SHELL_STATE_PAUSED,
	SHELL_STATE_CONNECTING,
	SHELL_STATE_SEARCHING,
	SHELL_STATE_LOADING,
	SHELL_STATE_BUSY,
	SHELL_STATE_IN_GAME
};

static const int MAX_SCREEN_AREAS = 32;

class idMenuScreen;

/*
================================================
idMenuHandler
================================================
*/
class idMenuHandler : public std::enable_shared_from_this<idMenuHandler> {
public:
	idMenuHandler();
	virtual ~idMenuHandler();
	virtual void Initialize(const std::string& filename);
	virtual void Cleanup();
	virtual void Update();
	virtual void UpdateChildren();
	virtual bool HandleGuiEvent(const sysEvent_t* sev);
	virtual bool IsActive();
	virtual void ActivateMenu(bool show);
	virtual int ActiveScreen() { return activeScreen; }

	virtual std::shared_ptr<GUI> GetGUI() { return gui; }
	virtual void AddChild(std::shared_ptr<idMenuWidget> widget);

protected:
	int activeScreen;
	int nextScreen;
	std::shared_ptr<GUI> gui;
	std::vector<std::shared_ptr<idMenuScreen>> menuScreens;
	std::vector<std::shared_ptr<idMenuWidget>> children;

	std::shared_ptr<idMenuWidget_CommandBar> cmdBar;
};

/*
================================================
idMenuHandler_Shell
================================================
*/
class idMenuHandler_Shell : public idMenuHandler {
public:
	idMenuHandler_Shell() :
		state(shellState_t::SHELL_STATE_INVALID),
		nextState(shellState_t::SHELL_STATE_INVALID),
		inGame(false) 
	{ }

	virtual void Update() override;
	virtual void ActivateMenu(bool show) override;
	virtual void Initialize(const std::string& filename) override;
	virtual void Cleanup() override;
	virtual bool HandleGuiEvent(const sysEvent_t* sev) override;

	void SetShellState(shellState_t s) { nextState = s; }

	void SetInGame(bool val) { inGame = val; }
	bool GetInGame() { return inGame; }
private:
	shellState_t state;
	shellState_t nextState;

	bool inGame;
};

#endif