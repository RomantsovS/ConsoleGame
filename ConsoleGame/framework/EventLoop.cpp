#include <precompiled.h>
#pragma hdrstop

constexpr int MAX_PUSHED_EVENTS = 64;

idEventLoop eventLoopLocal;
idEventLoop* eventLoop = &eventLoopLocal;

/*
=================
idEventLoop::idEventLoop
=================
*/
idEventLoop::idEventLoop() {
}

/*
=================
idEventLoop::~idEventLoop
=================
*/
idEventLoop::~idEventLoop() {
}

/*
=================
idEventLoop::GetRealEvent
=================
*/
sysEvent_t	idEventLoop::GetRealEvent() noexcept {
	sysEvent_t	ev;

	ev = Sys_GetEvent();

	return ev;
}

/*
=================
idEventLoop::GetEvent
=================
*/
sysEvent_t idEventLoop::GetEvent() noexcept {
	return GetRealEvent();
}

/*
=================
idEventLoop::ProcessEvent
=================
*/
void idEventLoop::ProcessEvent(sysEvent_t ev) {
	// track key up / down states
	if (ev.evType == SE_KEY) {
		idKeyInput::PreliminaryKeyEvent(ev.evValue, (ev.evValue2 != 0));
	}

	if (ev.evType == SE_CONSOLE) {
		// from a text console outside the game window
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, (char*)ev.evPtr);
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");
	}
	else {
		common->ProcessEvent(&ev);
	}

	// free any block data
	if (ev.evPtr) {
		//Mem_Free(ev.evPtr); TO_DO
	}
}

/*
===============
idEventLoop::RunEventLoop
===============
*/
int idEventLoop::RunEventLoop(bool commandExecution) {
	sysEvent_t	ev;

	while (1) {

		if (commandExecution) {
			// execute any bound commands before processing another event
			cmdSystem->ExecuteCommandBuffer();
		}

		ev = GetEvent();

		// if no more events are available
		if (ev.evType == SE_NONE) {
			return 0;
		}
		ProcessEvent(ev);
	}

	return 0;	// never reached
}

/*
=============
idEventLoop::Init
=============
*/
void idEventLoop::Init() noexcept {
}

/*
=============
idEventLoop::Shutdown
=============
*/
void idEventLoop::Shutdown() noexcept {
}