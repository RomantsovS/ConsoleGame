#ifndef D3XP_MENUS_MENUSCREEN_H__
#define D3XP_MENUS_MENUSCREEN_H__

#include "MenuWidget.h"

/*
================================================
idMenuScreen
================================================
*/
class idMenuScreen : public idMenuWidget {
public:

	idMenuScreen();
	virtual ~idMenuScreen();

	virtual void Update();

	virtual void ShowScreen();
	virtual void HideScreen();

protected:
	std::shared_ptr<GUI> menuGUI;
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
	virtual void Initialize(std::shared_ptr<idMenuHandler> data);
	virtual void Update();
	virtual void ShowScreen();
	virtual void HideScreen();
private:
	std::shared_ptr<idMenuWidget_Button> startButton;
};

//*
//================================================
//idMenuScreen_Shell_NewGame
//================================================
//*/
class idMenuScreen_Shell_NewGame : public idMenuScreen {
public:
	idMenuScreen_Shell_NewGame() { }
private:
};

#endif
 