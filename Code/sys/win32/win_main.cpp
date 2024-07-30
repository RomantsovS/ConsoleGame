#include "idlib/precompiled.h"

#include <io.h>

#include "win_local.h"
#include "../../renderer/tr_local.h"

Win32Vars_t win32;

#pragma optimize( "", on)

#ifdef DEBUG

static unsigned int debug_total_alloc = 0;
static unsigned int debug_total_alloc_count = 0;
static unsigned int debug_current_alloc = 0;
static unsigned int debug_current_alloc_count = 0;
static unsigned int debug_frame_alloc = 0;
static unsigned int debug_frame_alloc_count = 0;

idCVar sys_showMallocs("sys_showMallocs", "0", CVAR_SYSTEM, "");

#define  TIME_STR_LENGTH      10
#define  DATE_STR_LENGTH      10

FILE* logFile;                // Used to log allocation information
const char lineStr[] = { "---------------------------------------\
--------------------------------------\n" };

#define  FILE_IO_ERROR        0
#define  OUT_OF_MEMORY        1

// _HOOK_ALLOC, _HOOK_REALLOC, _HOOK_FREE

typedef struct CrtMemBlockHeader
{
	struct _CrtMemBlockHeader* pBlockHeaderNext;	// Pointer to the block allocated just before this one:
	struct _CrtMemBlockHeader* pBlockHeaderPrev;	// Pointer to the block allocated just after this one
	char* szFileName;    // File name
	int nLine;           // Line number
	size_t nDataSize;    // Size of user block
	int nBlockUse;       // Type of block
	long lRequest;       // Allocation number
	unsigned char gap[4];								// Buffer just before (lower than) the user's memory:
} CrtMemBlockHeader;

DEF_LOGS;

#ifdef MSVC

/*
==================
Sys_AllocHook

	called for every malloc/new/free/delete
==================
*/
int Sys_AllocHook(int nAllocType, void* pvData, size_t nSize, int nBlockUse, long lRequest, const unsigned char* szFileName, int nLine) {
	CrtMemBlockHeader* pHead;
	byte* temp;

	if (nBlockUse == _CRT_BLOCK) {
		return(TRUE);
	}

	// get a pointer to memory block header
	temp = (byte*)pvData;
	temp -= 32;
	pHead = (CrtMemBlockHeader*)temp;

	switch (nAllocType) {
	case	_HOOK_ALLOC:
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count++;
		debug_frame_alloc_count++;
		break;

	case	_HOOK_FREE:
		idassert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);

		debug_current_alloc -= pHead->nDataSize;
		debug_current_alloc_count--;
		debug_total_alloc_count++;
		debug_frame_alloc_count++;
		break;

	case	_HOOK_REALLOC:
		idassert(pHead->gap[0] == 0xfd && pHead->gap[1] == 0xfd && pHead->gap[2] == 0xfd && pHead->gap[3] == 0xfd);

		debug_current_alloc -= pHead->nDataSize;
		debug_total_alloc += nSize;
		debug_current_alloc += nSize;
		debug_frame_alloc += nSize;
		debug_total_alloc_count++;
		debug_current_alloc_count--;
		debug_frame_alloc_count++;
		break;
	}
	
	const char* operation[] = { "", "allocating", "re-allocating", "freeing" };
	const char* blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };
	
	_idassert((nAllocType > 0) && (nAllocType < 4));
	_idassert((nBlockUse >= 0) && (nBlockUse < 5));

	fprintf(logFile,
		"Memory operation in %s, line %d: %s a %d-byte '%s' block (#%ld)\n",
		szFileName, nLine, operation[nAllocType], nSize,
		blockType[nBlockUse], lRequest);
	if (pvData != NULL)
		fprintf(logFile, " at %p\n", pvData);

	return(TRUE);
}

#endif

/*
==================
Sys_DebugMemory_f
==================
*/
void Sys_DebugMemory_f() {
	common->Printf("Total allocation %8dk in %d blocks\n", debug_total_alloc / 1024, debug_total_alloc_count);
	common->Printf("Current allocation %8dk in %d blocks\n", debug_current_alloc / 1024, debug_current_alloc_count);
}

/*
==================
Sys_MemFrame
==================
*/
void Sys_MemFrame() {
	if (sys_showMallocs.GetInteger() > 0) {
		if(isCommonExists)
			common->Printf("Frame: %8dk in %5d blocks\n", debug_frame_alloc / 1024, debug_frame_alloc_count);
	}

	debug_frame_alloc = 0;
	debug_frame_alloc_count = 0;
}

#endif

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
Sys_Cwd
==============
*/
const char* Sys_Cwd() noexcept {
	static char cwd[MAX_OSPATH];

	_getcwd(cwd, sizeof(cwd) - 1);
	cwd[MAX_OSPATH - 1] = 0;

	return cwd;
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
void Sys_ClearEvents() noexcept {
	eventHead = eventTail = 0;
}

/*
================
Sys_GetEvent
================
*/
sysEvent_t Sys_GetEvent() noexcept {
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

BOOL WINAPI ConsoleHandler(DWORD CEvent) noexcept {
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
		//common->Quit();
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
	/*
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetDbgFlag(0);
	_CrtSetAllocHook(Sys_AllocHook);

	char timeStr[TIME_STR_LENGTH], dateStr[DATE_STR_LENGTH];         // Used to set up log file

	// Open a log file for the hook functions to use 
	fopen_s(&logFile, "mem.log", "w");
	if (logFile == nullptr)
		exit(FILE_IO_ERROR);
	_strtime_s(timeStr, TIME_STR_LENGTH);
	_strdate_s(dateStr, DATE_STR_LENGTH);
	fprintf(logFile,
		"Memory Allocation Log File for Example Program, run at %s on %s.\n",
		timeStr, dateStr);
	fputs(lineStr, logFile);
	*/
	//_CrtSetBreakAlloc(1143);
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

	common->Init(argc, argv, nullptr);

#ifdef DEBUG
	Sys_DebugMemory_f();
#endif

	// main game loop
	while (1) {
#ifdef DEBUG
		Sys_MemFrame();
#endif
		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}
