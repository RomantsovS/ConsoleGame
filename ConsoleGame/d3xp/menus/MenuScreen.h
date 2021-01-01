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
 