#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <direct.h>
#include <vector>

#include "../sys_public.h"
#include "../../framework/Common_local.h"
#include "win_local.h"
#include "../../framework/FileSystem.h"
#include "../../idlib/Str.h"

Win32Vars_t	win32;

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error(const char* error, ...) {
	va_list		argptr;
	char		text[4096];

	va_start(argptr, error);
	idStr::vsnPrintf(text, sizeof(text), error, argptr);
	va_end(argptr);

	MessageBox(NULL, TEXT(text), TEXT("Sys Error"), MB_OK);
	exit(EXIT_FAILURE);
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit() {
	//exit(EXIT_SUCCESS);
	ExitProcess(0);
}

/*
==============
Sys_Sleep
==============
*/
void Sys_Sleep(int msec) {
	Sleep(msec);
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir(const std::string &path) {
	_mkdir(path.c_str());
}

/*
==============
Sys_Cwd
==============
*/
const char* Sys_Cwd() {
	static char cwd[MAX_OSPATH];

	_getcwd(cwd, sizeof(cwd) - 1);
	cwd[MAX_OSPATH - 1] = 0;

	return cwd;
}

/*
==============
Sys_DefaultBasePath
==============
*/
const char* Sys_DefaultBasePath() {
	return Sys_Cwd();
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

constexpr size_t MAX_QUED_EVENTS = 256;
constexpr size_t MASK_QUED_EVENTS = (MAX_QUED_EVENTS - 1);

std::vector<sysEvent_t> eventQue(MAX_QUED_EVENTS);
int			eventHead = 0;
int			eventTail = 0;

/*
================
Sys_QueEvent

Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Sys_QueEvent(sysEventType_t type, int value, int value2, int ptrLength, void* ptr, int inputDeviceNum) {
	sysEvent_t* ev = &eventQue[eventHead & MASK_QUED_EVENTS];

	if (eventHead - eventTail >= MAX_QUED_EVENTS) {
		common->Printf("Sys_QueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if (ev->evPtr) {
			//Mem_Free(ev->evPtr); TO_DO
		}
		eventTail++;
	}

	eventHead++;

	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
	ev->inputDevice = inputDeviceNum;
}

/*
================
Sys_ClearEvents
================
*/
void Sys_ClearEvents() {
	eventHead = eventTail = 0;
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent() {
	sysEvent_t	ev;

	// return if we have data
	if (eventHead > eventTail) {
		eventTail++;
		return eventQue[(eventTail - 1) & MASK_QUED_EVENTS];
	}

	// return the empty event 
	memset(&ev, 0, sizeof(ev));

	return ev;
}

int main(int argc, const char * const * argv, const char * cmdline)
{
	// get the initial time base
	Sys_Milliseconds();

	common->Init(argc, argv, cmdline);

	// main game loop
	while (1)
	{
		// run the game
		common->Frame();
	}

	return 0;
}