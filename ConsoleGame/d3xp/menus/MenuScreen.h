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

	virtual void Update() override;
	virtual void UpdateCmds();

	virtual void ShowScreen();
	virtual void HideScreen();

protected:
	std::shared_ptr<idSWF> menuGUI;
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
	virtual void				Initialize(std::shared_ptr<idMenuHandler> data) override;
	virtual void				Update() override;
	virtual void				ShowScreen() override;
	virtual void				HideScreen() override;
	virtual bool				HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false) override;

	void						HandleExitGameBtn();
	int							GetRootIndex();
	void						SetRootIndex(int index);

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
	virtual void Initialize(std::shared_ptr<idMenuHandler> data);
	virtual void Update();
	virtual void ShowScreen();
	virtual void HideScreen();
	virtual bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false);

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
	idMenuScreen_Shell_PressStart() :
	startButton(nullptr) {
	}
	virtual void Initialize(std::shared_ptr<idMenuHandler> data) override;
	virtual void Update() override;
	virtual void ShowScreen() override;
	virtual void HideScreen() override;
	virtual bool HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false) override;
private:
	std::shared_ptr<idMenuWidget_Button> startButton;
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
	virtual void				Initialize(std::shared_ptr<idMenuHandler> data) override;
	virtual void				Update() override;
	virtual void				ShowScreen() override;
	virtual void				HideScreen() override;
	virtual bool				HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false) override;

	void						SetCanContinue(bool valid) { canContinue = valid; }
	void						ContinueGame();
private:
	bool						canContinue;
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
	virtual void				Initialize(std::shared_ptr<idMenuHandler> data);
	virtual void				Update();
	virtual void				ShowScreen();
	virtual void				HideScreen();
	virtual bool				HandleAction(idWidgetAction& action, const idWidgetEvent& event, std::shared_ptr<idMenuWidget> widget, bool forceHandled = false);
private:
	std::shared_ptr<idMenuWidget_DynamicList> options;
	std::shared_ptr<idMenuWidget_Button> btnBack;
};

#endif
 