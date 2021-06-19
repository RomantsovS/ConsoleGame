#pragma hdrstop
#include "../../idlib/precompiled.h"

#include <io.h>

#include "win_local.h"
#include "../../renderer/tr_local.h"

Win32Vars_t win32;

#pragma optimize( "", on)

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
Sys_Printf
==============
*/
#define MAXPRINTMSG 4096
void Sys_Printf(const char* fmt, ...) {
	char		msg[MAXPRINTMSG];

	va_list argptr;
	va_start(argptr, fmt);
	idStr::vsnPrintf(msg, MAXPRINTMSG - 1, fmt, argptr);
	va_end(argptr);
	msg[sizeof(msg) - 1] = '\0';

	OutputDebugString(msg);
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
==============
Sys_ListFiles
==============
*/
int Sys_ListFiles(const std::string& directory, std::string extension, std::vector<std::string>& list) {
	std::string search;
	struct _finddata_t findinfo;
	int			findhandle;
	int			flag;

	if (extension.empty()) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if (extension[0] == '/' && extension[1] == 0) {
		extension = "";
		flag = 0;
	}
	else {
		flag = _A_SUBDIR;
	}

	sprintf(search, "%s\\*%s", directory.c_str(), extension.c_str());

	// search
	list.clear();

	findhandle = _findfirst(search.c_str(), &findinfo);
	if (findhandle == -1) {
		return -1;
	}

	do {
		if (flag ^ (findinfo.attrib & _A_SUBDIR)) {
			list.push_back(findinfo.name);
		}
	} while (_findnext(findhandle, &findinfo) != -1);

	_findclose(findhandle);

	return list.size();
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

BOOL WINAPI ConsoleHandler(DWORD CEvent) {
	switch (CEvent)
	{
	case CTRL_C_EVENT:
		MessageBox(NULL,
			"CTRL+C received!", "CEvent", MB_OK);
		break;
	case CTRL_BREAK_EVENT:
		MessageBox(NULL,
			"CTRL+BREAK received!", "CEvent", MB_OK);
		break;
	case CTRL_CLOSE_EVENT:
		common->Quit();
		break;
	case CTRL_LOGOFF_EVENT:
		MessageBox(NULL,
			"User is logging off!", "CEvent", MB_OK);
		break;
	case CTRL_SHUTDOWN_EVENT:
		MessageBox(NULL,
			"User is logging off!", "CEvent", MB_OK);
		break;

	}
	return TRUE;
}

int main(int argc, const char * const * argv) {
#ifdef DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(13910);
#endif

	if (SetConsoleCtrlHandler(
		(PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
		// unable to install handler... 
		// display message to the user
		printf("Unable to install handler!\n");
		return -1;
	}

	// get the initial time base
	Sys_Milliseconds();

	//common->Init(argc, argv, nullptr);

	// main game loop
	while (1)
	{
		// run the game
		//common->Frame();
		Sys_Sleep(1000);
	}

	common->Shutdown();

	return 0;
}
