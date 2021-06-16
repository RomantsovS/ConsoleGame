#pragma hdrstop
#include "../idlib/precompiled.h"
#include "Common_local.h"

// the console will query the cvar and command systems for
// command completion information

class idConsoleLocal : public idConsole {
public:
	virtual	void		Init() override;
	virtual void		Shutdown() override;
	virtual	bool		ProcessEvent(const sysEvent_t* event, bool forceAccept) override;
	virtual	bool		Active() override;
	//virtual	void		ClearNotifyLines();
	virtual	void		Close() override;
	//virtual	void		Print(const char* text);
	virtual	void		Draw(bool forceFullScreen);

	//void				Dump(const char* toFile);
	void				Clear();

private:
	void				KeyDownEvent(int key);

	/*void				Linefeed();

	void				PageUp();
	void				PageDown();
	void				Top();
	void				Bottom();

	void				DrawInput();
	void				DrawNotify();
	void				DrawSolidConsole(float frac);

	void				Scroll();
	void				SetDisplayFraction(float frac);
	void				UpdateDisplayFraction();

	void				DrawTextLeftAlign(float x, float& y, const char* text, ...);
	void				DrawTextRightAlign(float x, float& y, const char* text, ...);
*/
	//float				DrawMemoryUsage(float y);

	//============================

	bool keyCatching;

	idEditField consoleField;
};

static idConsoleLocal localConsole;
idConsole* console = &localConsole;

/*
==============
idConsoleLocal::Init
==============
*/
void idConsoleLocal::Init() {
	keyCatching = false;
	consoleField.Clear();
}

/*
==============
idConsoleLocal::Shutdown
==============
*/
void idConsoleLocal::Shutdown() {;
}

/*
================
idConsoleLocal::Active
================
*/
bool idConsoleLocal::Active() {
	return keyCatching;
}

/*
================
idConsoleLocal::Close
================
*/
void idConsoleLocal::Close() {
	keyCatching = false;
}

/*
================
idConsoleLocal::Clear
================
*/
void idConsoleLocal::Clear() {
}

/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
KeyDownEvent

Handles history and console scrollback
====================
*/
void idConsoleLocal::KeyDownEvent(int key) {

	// Execute F key bindings
	if (key >= static_cast<int>(keyNum_t::K_F1) && key <= static_cast<int>(keyNum_t::K_F12)) {
		idKeyInput::ExecKeyBinding(key);
		return;
	}

	// ctrl-L clears screen
	if (key == static_cast<int>(keyNum_t::K_L) && (idKeyInput::IsDown(static_cast<int>(keyNum_t::K_LCTRL)) || idKeyInput::IsDown(static_cast<int>(keyNum_t::K_RCTRL)))) {
		Clear();
		return;
	}

	// enter finishes the line
	if (key == static_cast<int>(keyNum_t::K_ENTER) || key == static_cast<int>(keyNum_t::K_KP_ENTER)) {

		common->Printf(">%s\n", consoleField.GetBuffer());

		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, consoleField.GetBuffer());	// valid command
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");

		// copy line to history buffer

		consoleField.Clear();
		return;
	}

	// pass to the normal editline routine
	consoleField.KeyDownEvent(key);
}

/*
==============
ProcessEvent
==============
*/
bool idConsoleLocal::ProcessEvent(const sysEvent_t* proc_event, bool forceAccept) {
	const bool consoleKey = proc_event->evType == SE_CHAR && proc_event->evValue == static_cast<int>(keyNum_t::K_GRAVE) && com_allowConsole.GetBool();

	// we always catch the console key event
	if (!forceAccept && consoleKey) {
		// ignore up events
		if (proc_event->evValue2 == 0) {
			return true;
		}

		// a down event will toggle the destination lines
		if (keyCatching) {
			Close();
		}
		else {
			consoleField.Clear();
			keyCatching = true;
		}
		return true;
	}

	// if we aren't key catching, dump all the other events
	if (!forceAccept && !keyCatching) {
		return false;
	}

	// handle key and character events
	if (proc_event->evType == SE_CHAR) {
		// never send the console key as a character
		if (proc_event->evValue != '`' && proc_event->evValue != '~'
			&& proc_event->evValue2 == 1) {
			consoleField.CharEvent(proc_event->evValue);
		}
		return true;
	}

	if (proc_event->evType == SE_KEY) {
		// ignore up key events
		if (proc_event->evValue2 == 0) {
			return true;
		}

		KeyDownEvent(proc_event->evValue);
		return true;
	}

	// we don't handle things like mouse, joystick, and network packets
	return false;
}

/*
==============
Draw

ForceFullScreen is used by the editor
==============
*/
void idConsoleLocal::Draw(bool forceFullScreen) {
	if (forceFullScreen) {
		// if we are forced full screen because of a disconnect, 
		// we want the console closed when we go back to a session state
		Close();
		// we are however catching keyboard input
		keyCatching = true;
	}

	if (keyCatching /*&& tr.update_info*/) {
		std::string console_text = std::string(":").append(consoleField.GetBuffer());
		console_text.append("_");

		renderSystem->DrawString(console_text, Screen::ConsoleColor::White);
	}
}