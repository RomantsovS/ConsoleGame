#include "idlib/precompiled.h"


/*
===================
idSWF::HandleEvent
===================
*/
bool idSWF::HandleEvent(const sysEvent_t* event) {
	if (!IsActive()) {
		return false;
	}

	if (event->evType == SE_KEY) {
		const std::string& keyName = idKeyInput::KeyNumToString((keyNum_t)event->evValue);
		idSWFScriptVar var = shortcutKeys->Get(keyName);
		// anything more than 32 levels of indirection we can be pretty sure is an infinite loop
		for (int runaway = 0; runaway < 32; runaway++) {
			idSWFParmList eventParms;
			eventParms.clear();
			eventParms.Append(event->inputDevice);
			if (var.IsString()) {
				// alias to another key
				var = shortcutKeys->Get(var.ToString());
				continue;
			}
			else if (var.IsObject()) {
				// if this object is a sprite, send fake mouse events to it
				std::shared_ptr<idSWFScriptObject> object = var.GetObjectScript();
				// make sure we don't send an onRelease event unless we have already sent that object an onPress
				bool wasPressed = object->Get("_pressed").ToBool();
				object->Set("_pressed", event->evValue2);
				if (event->evValue2) {
					var = object->Get("onPress");
				}
				else if (wasPressed) {
					var = object->Get("onRelease");
				}
				if (var.IsFunction()) {
					var.GetFunction()->Call(object.get(), eventParms);
					return true;
				}
			}
			else if (var.IsFunction()) {
				if (event->evValue2) {
					// anonymous functions only respond to key down events
					var.GetFunction()->Call(nullptr, eventParms);
					return true;
				}
				return false;
			}

			/*idSWFScriptVar useFunction = globals->Get("useFunction");
			if (useFunction.IsFunction() && event->evValue2) {
				const char* action = idKeyInput::GetBinding(event->evValue);
				if (idStr::Cmp("_use", action) == 0) {
					useFunction.GetFunction()->Call(NULL, idSWFParmList());
				}
			}

			idSWFScriptVar waitInput = globals->Get("waitInput");
			if (waitInput.IsFunction()) {
				useMouse = false;
				if (event->evValue2) {
					idSWFParmList waitParms;
					waitParms.Append(event->evValue);
					waitInput.GetFunction()->Call(NULL, waitParms);
				}
			}
			else {
				useMouse = true;
			}*/

			/*idSWFScriptVar focusWindow = globals->Get("focusWindow");
			if (focusWindow.IsObject()) {
				idSWFScriptVar onKey = focusWindow.GetObject()->Get("onKey");
				if (onKey.IsFunction()) {

					// make sure we don't send an onRelease event unless we have already sent that object an onPress
					idSWFScriptObject* object = focusWindow.GetObject();
					bool wasPressed = object->Get("_kpressed").ToBool();
					object->Set("_kpressed", event->evValue2);
					if (event->evValue2 || wasPressed) {
						idSWFParmList parms;
						parms.Append(event->evValue);
						parms.Append(event->evValue2);
						onKey.GetFunction()->Call(focusWindow.GetObject(), parms).ToBool();
						return true;
					}
					else if (event->evValue == K_LSHIFT || event->evValue == K_RSHIFT) {
						idSWFParmList parms;
						parms.Append(event->evValue);
						parms.Append(event->evValue2);
						onKey.GetFunction()->Call(focusWindow.GetObject(), parms).ToBool();
					}
				}
			}*/
			return false;
		}
		common->Warning("Circular reference in %s shortcutKeys.%s", filename.c_str(), keyName);
	}
	else if (event->evType == SE_CHAR) {
	}
	return false;
}
