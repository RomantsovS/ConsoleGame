#ifndef FRAMEWORK_CONSOLE_H_
#define FRAMEWORK_CONSOLE_H_

#include "../sys/sys_public.h"

/*
===============================================================================

	The console is strictly for development and advanced users. It should
	never be used to convey actual game information to the user, which should
	always be done through a GUI.

	The force options are for the editor console display window, which
	doesn't respond to pull up / pull down

===============================================================================
*/

class idConsole {
public:
	virtual			~idConsole() {}

	virtual void	Init() = 0;
	virtual void	Shutdown() = 0;

	virtual bool	ProcessEvent(const sysEvent_t* event, bool forceAccept) = 0;

	// the system code can release the mouse pointer when the console is active
	virtual bool	Active() = 0;

	// clear the timers on any recent prints that are displayed in the notify lines
	//virtual void	ClearNotifyLines() = 0;

	// some console commands, like timeDemo, will force the console closed before they start
	virtual void	Close() = 0;

	virtual void	Draw(bool forceFullScreen) = 0;
	//virtual void	Print(const char* text) = 0;
};

extern idConsole* console;	// statically initialized to an idConsoleLocal

#endif