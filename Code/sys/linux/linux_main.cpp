#include "idlib/precompiled.h"

#include "../../renderer/tr_local.h"

int main(int argc, const char * const * argv) {
	// get the initial time base
	Sys_Milliseconds();

	common->Init(argc, argv, nullptr);

	// main game loop
	while (1) {
		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}

void Sys_Error(const char* error, ...) {
	exit(EXIT_FAILURE);
}

void Sys_ClearEvents() noexcept {
}

void Sys_Printf(const char* fmt, ...) {
}

sysEvent_t Sys_GetEvent() noexcept {
	sysEvent_t	ev;

	// return the empty event 
	memset(&ev, 0, sizeof(ev));

	return ev;
}

int Sys_PollKeyboardInputEvents() noexcept {
	return 0;
}

int Sys_ReturnKeyboardInputEvent(const int n, int& ch, bool& state) {
	return 0;
}

void Sys_InitInput() {
	common->Printf("------- Input Initialization -------\n");

	common->Printf("------------------------------------\n");
}