#ifndef D3XP_MENUS_MENUSCREEN_H__
#define D3XP_MENUS_MENUSCREEN_H__

#include "../../renderer/tr_local.h"

/*
================================================
idMenuScreen
================================================
*/
class idMenuScreen : public idMenuWidget {
public:

	idMenuScreen();
	virtual ~idMenuScreen();
	idMenuScreen(const idMenuScreen&) = default;
	idMenuScreen& operator=(const idMenuScreen&) = default;
	idMenuScreen(idMenuScreen&&) = default;
	idMenuScreen& operator=(idMenuScreen&&) = default;

	void Update() noexcept override;
	virtual void UpdateCmds();

	virtual void ShowScreen();
	virtual void HideScreen();

protected:
	std::weak_ptr<idSWF> menuGUI;
};

//*
//================================================	
//idMenuScreen_Shell_Root
//================================================
//*/
class idMenuScreen_Shell_Root : public idMenuScreen {
public:
	idMenuScreen_Shell_Root() :
		options(nullptr) {
	}
	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;

	void HandleExitGameBtn();
	int GetRootIndex() noexcept;
	void SetRootIndex(int index);

private:
	std::shared_ptr<idMenuWidget_DynamicList> options;
};

//*
//================================================	
//idMenuScreen_Shell_Pause
//================================================
//*/
class idMenuScreen_Shell_Pause : public idMenuScreen {
public:
	idMenuScreen_Shell_Pause() :
		options(nullptr),
		isMpPause(false) {
	}
	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;

	void HandleExitGameBtn();
	void HandleRestartBtn();

private:
	std::shared_ptr<idMenuWidget_DynamicList> options;
	bool						isMpPause;
};

//*
//================================================
//idMenuScreen_Shell_PressStart
//================================================
//*/
class idMenuScreen_Shell_PressStart : public idMenuScreen {
public:
	idMenuScreen_Shell_PressStart() = default;

	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;
};

//*
//================================================	
//idMenuScreen_Shell_Singleplayer
//================================================
//*/
class idMenuScreen_Shell_Singleplayer : public idMenuScreen {
public:
	idMenuScreen_Shell_Singleplayer() :
		options(nullptr),
		btnBack(nullptr),
		canContinue(false) {
	}
	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;

	void SetCanContinue(bool valid) noexcept { canContinue = valid; }
	void ContinueGame() noexcept;
private:
	bool canContinue;
	std::shared_ptr<idMenuWidget_DynamicList> options;
	std::shared_ptr<idMenuWidget_Button> btnBack;
};

//*
//================================================	
//idMenuScreen_Shell_NewGame
//================================================
//*/
class idMenuScreen_Shell_NewGame : public idMenuScreen {
public:
	idMenuScreen_Shell_NewGame() :
		options(nullptr),
		btnBack(nullptr) {
	}
	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;
private:
	std::shared_ptr<idMenuWidget_DynamicList> options;
	std::shared_ptr<idMenuWidget_Button> btnBack;
};

//*
//================================================	
//idMenuScreen_Shell_PartyLobby
//================================================
//*/
class idMenuScreen_Shell_PartyLobby : public idMenuScreen {
public:
	idMenuScreen_Shell_PartyLobby() :
		options(nullptr),
		lobby(nullptr),
		isHost(false),
		isPeer(false),
		btnBack(nullptr),
		inParty(false) {
	}

	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;

	void UpdateOptions();
	void UpdateLobby();

private:

	bool isHost;
	bool isPeer;
	bool inParty;
	std::shared_ptr<idMenuWidget_DynamicList> options;
	std::unique_ptr<idMenuWidget_LobbyList> lobby;
	std::shared_ptr<idMenuWidget_Button> btnBack;

	std::vector<std::vector<std::string>> menuOptions;
};

//*
//================================================	
//idMenuScreen_Shell_GameLobby
//================================================
//*/
class idMenuScreen_Shell_GameLobby : public idMenuScreen {
public:
	idMenuScreen_Shell_GameLobby() :
		options(nullptr),
		lobby(nullptr),
		isPeer(false),
		isHost(false),
		privateGameLobby(true),
		btnBack(nullptr) {
	}

	void Initialize(std::shared_ptr<idMenuHandler> data) noexcept override;
	void Update() noexcept override;
	void ShowScreen() override;
	void HideScreen() override;
	bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, idMenuWidget* widget, bool forceHandled = false) override;
	void UpdateLobby();
	//bool CanKickSelectedPlayer(lobbyUserID_t& luid);

private:

	bool isHost;
	bool isPeer;
	bool privateGameLobby;

	std::shared_ptr<idMenuWidget_DynamicList> options;
	std::unique_ptr<idMenuWidget_LobbyList> lobby;
	std::shared_ptr<idMenuWidget_Button> btnBack;
	std::vector<std::vector<std::string>> menuOptions;
};

#endif
 