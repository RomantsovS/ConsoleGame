#include "GUI.h"

#include "../framework/KeyInput.h"

/*
===================
idSWF::HandleEvent
===================
*/
bool GUI::HandleEvent(const sysEvent_t* event) {
	if (!IsActive()) {
		return false;
	}

	if (event->evType == SE_KEY) {
		const std::string& keyName = idKeyInput::KeyNumToString((keyNum_t)event->evValue);
		GUIScriptVar var = shortcutKeys->Get(keyName);
	}
	else if (event->evType == SE_CHAR) {
	}
	return false;
}
