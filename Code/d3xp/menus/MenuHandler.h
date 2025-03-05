#ifndef D3XP_MENUS_MENUHANDLER_H__
#define D3XP_MENUS_MENUHANDLER_H__

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

static constexpr int MAX_SCREEN_AREAS = 32;
static constexpr int DEFAULT_REPEAT_TIME = 150;
static const int WAIT_START_TIME_LONG = 30000;

struct actionRepeater_t {
  actionRepeater_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
    common->DPrintf("%s ctor\n", "actionRepeater_t");
#endif
  }
  ~actionRepeater_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
    common->DPrintf("%s dtor\n", "actionRepeater_t");
#endif
  }
  actionRepeater_t(const actionRepeater_t&) = default;
  actionRepeater_t& operator=(const actionRepeater_t&) = default;
  actionRepeater_t(actionRepeater_t&&) = default;
  actionRepeater_t& operator=(actionRepeater_t&&) = default;

  idMenuWidget* widget = nullptr;
  idWidgetEvent event;
  idWidgetAction action;
  int numRepetitions{};
  int nextRepeatTime{};
  int repeatDelay{DEFAULT_REPEAT_TIME};
  int screenIndex{-1};
  bool isActive{};
};

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
  idMenuHandler(const idMenuHandler&) = default;
  idMenuHandler& operator=(const idMenuHandler&) = default;
  idMenuHandler(idMenuHandler&&) = default;
  idMenuHandler& operator=(idMenuHandler&&) = default;

  virtual void Initialize(const std::string& filename);
  virtual void Cleanup() noexcept;
  virtual void Update();
  virtual void UpdateChildren() noexcept;
  virtual bool HandleGuiEvent(const sysEvent_t* sev);
  virtual bool IsActive() noexcept;
  virtual void ActivateMenu(bool show) noexcept;
  virtual bool HandleAction(idWidgetAction& action, const idWidgetEvent& event,
                            idMenuWidget* widget, bool forceHandled = false);
  virtual int ActiveScreen() noexcept { return activeScreen; }
  virtual int NextScreen() { return nextScreen; }
  virtual void SetNextScreen(shellAreas_t screen) noexcept {
    nextScreen = static_cast<int>(screen);
  }

  virtual void StartWidgetActionRepeater(idMenuWidget* widget,
                                         const idWidgetAction& action,
                                         const idWidgetEvent& event);
  virtual void PumpWidgetActionRepeater();
  virtual void ClearWidgetActionRepeater() noexcept;
  virtual std::shared_ptr<idSWF> GetGUI() noexcept { return gui; }
  virtual void AddChild(std::shared_ptr<idMenuWidget> widget);

  std::shared_ptr<idMenuWidget_CommandBar> GetCmdBar() noexcept {
    return cmdBar;
  }

 protected:
  int activeScreen;
  int nextScreen;
  std::shared_ptr<idSWF> gui;
  actionRepeater_t actionRepeater;
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
  idMenuHandler_Shell()
      : state(shellState_t::SHELL_STATE_INVALID),
        nextState(shellState_t::SHELL_STATE_INVALID),
        newGameType(0),
        inGame(false),
        menuBar(nullptr) {}

  void Update() override;
  void ActivateMenu(bool show) noexcept override;
  void Initialize(const std::string& filename) override;
  void Cleanup() noexcept override;
  bool HandleAction(idWidgetAction& action, const idWidgetEvent& event,
                    idMenuWidget* widget, bool forceHandled = false) override;
  bool HandleGuiEvent(const sysEvent_t* sev) override;

  void SetShellState(shellState_t s) noexcept { nextState = s; }

  std::shared_ptr<idMenuWidget_MenuBar> GetMenuBar() noexcept {
    return menuBar;
  }

  void SetTimeRemaining(int time) { timeRemaining = time; }
  void SetNewGameType(int type) noexcept { newGameType = type; }
  int GetNewGameType() noexcept { return newGameType; }
  void SetInGame(bool val) noexcept { inGame = val; }
  bool GetInGame() noexcept { return inGame; }
  void HandleExitGameBtn();
  void SetupPCOptions();
  void UpdateLobby(idMenuWidget_LobbyList* lobbyList);
  void StartGame(int index);

 private:
  shellState_t state;
  shellState_t nextState;

  std::shared_ptr<idMenuWidget_MenuBar> menuBar;
  int timeRemaining;
  int newGameType;
  bool inGame;
  std::vector<std::string> navOptions;
};

#endif