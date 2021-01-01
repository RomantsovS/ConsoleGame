#ifndef D3XP_MENUS_MENUWIDGET_H__
#define D3XP_MENUS_MENUWIDGET_H__

#include "MenuHandler.h"

class idMenuHandler;

/*
================================================
idMenuWidget

We're using a model/view architecture, so this is the combination of both model and view.  The
other part of the view is the SWF itself.
================================================
*/
class idMenuWidget {
public:
	idMenuWidget();

	virtual ~idMenuWidget();

	// typically this is where the allocations for a widget will occur: sub widgets, etc.
	// Note that SWF sprite objects may not be accessible at this point.
	virtual void Initialize(std::shared_ptr<idMenuHandler> data) { menuData = data; }
protected:
	std::shared_ptr<idMenuHandler> menuData;
};

#endif
