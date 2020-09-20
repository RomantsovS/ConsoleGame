#include <Windows.h>

#include "win_local.h"
#include "../../framework/Common_local.h"

bool IN_StartupKeyboard() {
    win32.h_console_std_in = GetStdHandle(STD_INPUT_HANDLE);

    if (win32.h_console_std_in == INVALID_HANDLE_VALUE)
        common->FatalError("CreateConsoleScreenBuffer  failed - (%d)\n", GetLastError());

    common->Printf("keyboard: win input initialized.\n");
    return true;
}

/*
===========
Sys_ShutdownInput
===========
*/
void Sys_ShutdownInput() {
}

/*
===========
Sys_InitInput
===========
*/
void Sys_InitInput() {
	common->Printf("------- Input Initialization -------\n");

	IN_StartupKeyboard();

	common->Printf("------------------------------------\n");
}

/*
====================
Sys_PollKeyboardInputEvents
====================
*/
int Sys_PollKeyboardInputEvents() {
    DWORD events;

    GetNumberOfConsoleInputEvents(win32.h_console_std_in, &events);

	if (!events)
		return 0;

	return events;
}

/*
====================
Sys_PollKeyboardInputEvents
====================
*/
int Sys_ReturnKeyboardInputEvent(const int n, int& ch, bool& state) {
	INPUT_RECORD input_record;
	DWORD events;

	ch = 0;

	ReadConsoleInput(win32.h_console_std_in, &input_record, 1, &events);

	switch (input_record.EventType)
	{
	case KEY_EVENT:
	{
		ch = input_record.Event.KeyEvent.wVirtualKeyCode;

		if (input_record.Event.KeyEvent.bKeyDown)
		{
			state = true;
		}
		else
			state = false;

		if ((input_record.Event.KeyEvent.uChar.AsciiChar >= 'A' &&
			input_record.Event.KeyEvent.uChar.AsciiChar <= 'Z') ||
			(input_record.Event.KeyEvent.uChar.AsciiChar >= 'a' &&
			input_record.Event.KeyEvent.uChar.AsciiChar <= 'z')) {
			Sys_QueEvent(SE_CHAR, ch, state, 0, NULL, 0);
		}
		else
			Sys_QueEvent(SE_KEY, ch, state, 0, NULL, 0);
	}
	}
	
	return ch;
}