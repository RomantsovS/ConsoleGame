#include <vector>

#include "KeyInput.h"
#include "../sys/sys_public.h"
#include "CmdSystem.h"
#include "Common_local.h"
#include "UsercmdGen.h"
#include "../idlib/Lib.h"

struct keyname_t {
	keyNum_t keynum;
	const std::string name;
	const std::string strId;	// localized string id
};

#define NAMEKEY( code, strId ) { K_##code, #code, strId }
#define NAMEKEY2( code ) { K_##code, #code, #code }

#define ALIASKEY( alias, code ) { K_##code, alias, "" }

// names not in this list can either be lowercase ascii, or '0xnn' hex sequences
std::vector<keyname_t> keynames = {
	keyname_t({keyNum_t::K_SPACE, "SPACE", "#str_07021"}),
	keyname_t({keyNum_t::K_TAB, "TAB", "#str_07018"}),
	keyname_t({keyNum_t::K_F1, "F1", "#str_07018"}),
	keyname_t({keyNum_t::K_UPARROW, "UPARROW", "#str_07023"})
};

/*std::vector<keyname_t> keynames = {
	NAMEKEY(ESCAPE, "#str_07020"),
	NAMEKEY2(1),
	NAMEKEY2(2),
	NAMEKEY2(3),
	NAMEKEY2(4),
	NAMEKEY2(5),
	NAMEKEY2(6),
	NAMEKEY2(7),
	NAMEKEY2(8),
	NAMEKEY2(9),
	NAMEKEY2(0),
	NAMEKEY(MINUS, "-"),
	NAMEKEY(EQUALS, "="),
	NAMEKEY(BACKSPACE, "#str_07022"),
	NAMEKEY(TAB, "#str_07018"),
	NAMEKEY2(Q),
	NAMEKEY2(W),
	NAMEKEY2(E),
	NAMEKEY2(R),
	NAMEKEY2(T),
	NAMEKEY2(Y),
	NAMEKEY2(U),
	NAMEKEY2(I),
	NAMEKEY2(O),
	NAMEKEY2(P),
	NAMEKEY(LBRACKET, "["),
	NAMEKEY(RBRACKET, "]"),
	NAMEKEY(ENTER, "#str_07019"),
	NAMEKEY(LCTRL, "#str_07028"),
	NAMEKEY2(A),
	NAMEKEY2(S),
	NAMEKEY2(D),
	NAMEKEY2(F),
	NAMEKEY2(G),
	NAMEKEY2(H),
	NAMEKEY2(J),
	NAMEKEY2(K),
	NAMEKEY2(L),
	NAMEKEY(SEMICOLON, "#str_07129"),
	NAMEKEY(APOSTROPHE, "#str_07130"),
	NAMEKEY(GRAVE, "`"),
	NAMEKEY(LSHIFT, "#str_07029"),
	NAMEKEY(BACKSLASH, "\\"),
	NAMEKEY2(Z),
	NAMEKEY2(X),
	NAMEKEY2(C),
	NAMEKEY2(V),
	NAMEKEY2(B),
	NAMEKEY2(N),
	NAMEKEY2(M),
	NAMEKEY(COMMA, ","),
	NAMEKEY(PERIOD, "."),
	NAMEKEY(SLASH, "/"),
	NAMEKEY(RSHIFT, "#str_bind_RSHIFT"),
	NAMEKEY(KP_STAR, "#str_07126"),
	NAMEKEY(LALT, "#str_07027"),
	NAMEKEY(SPACE, "#str_07021"),
	NAMEKEY(CAPSLOCK, "#str_07034"),
	NAMEKEY(F1, "#str_07036"),
	NAMEKEY(F2, "#str_07037"),
	NAMEKEY(F3, "#str_07038"),
	NAMEKEY(F4, "#str_07039"),
	NAMEKEY(F5, "#str_07040"),
	NAMEKEY(F6, "#str_07041"),
	NAMEKEY(F7, "#str_07042"),
	NAMEKEY(F8, "#str_07043"),
	NAMEKEY(F9, "#str_07044"),
	NAMEKEY(F10, "#str_07045"),
	NAMEKEY(NUMLOCK, "#str_07125"),
	NAMEKEY(SCROLL, "#str_07035"),
	NAMEKEY(KP_7, "#str_07110"),
	NAMEKEY(KP_8, "#str_07111"),
	NAMEKEY(KP_9, "#str_07112"),
	NAMEKEY(KP_MINUS, "#str_07123"),
	NAMEKEY(KP_4, "#str_07113"),
	NAMEKEY(KP_5, "#str_07114"),
	NAMEKEY(KP_6, "#str_07115"),
	NAMEKEY(KP_PLUS, "#str_07124"),
	NAMEKEY(KP_1, "#str_07116"),
	NAMEKEY(KP_2, "#str_07117"),
	NAMEKEY(KP_3, "#str_07118"),
	NAMEKEY(KP_0, "#str_07120"),
	NAMEKEY(KP_DOT, "#str_07121"),
	NAMEKEY(F11, "#str_07046"),
	NAMEKEY(F12, "#str_07047"),
	NAMEKEY2(F13),
	NAMEKEY2(F14),
	NAMEKEY2(F15),
	NAMEKEY2(KANA),
	NAMEKEY2(CONVERT),
	NAMEKEY2(NOCONVERT),
	NAMEKEY2(YEN),
	NAMEKEY(KP_EQUALS, "#str_07127"),
	NAMEKEY2(CIRCUMFLEX),
	NAMEKEY(AT, "@"),
	NAMEKEY(COLON, ":"),
	NAMEKEY(UNDERLINE, "_"),
	NAMEKEY2(KANJI),
	NAMEKEY2(STOP),
	NAMEKEY2(AX),
	NAMEKEY2(UNLABELED),
	NAMEKEY(KP_ENTER, "#str_07119"),
	NAMEKEY(RCTRL, "#str_bind_RCTRL"),
	NAMEKEY(KP_COMMA, ","),
	NAMEKEY(KP_SLASH, "#str_07122"),
	NAMEKEY(PRINTSCREEN, "#str_07179"),
	NAMEKEY(RALT, "#str_bind_RALT"),
	NAMEKEY(PAUSE, "#str_07128"),
	NAMEKEY(HOME, "#str_07052"),
	NAMEKEY(UPARROW, "#str_07023"),
	NAMEKEY(PGUP, "#str_07051"),
	NAMEKEY(LEFTARROW, "#str_07025"),
	NAMEKEY(RIGHTARROW, "#str_07026"),
	NAMEKEY(END, "#str_07053"),
	NAMEKEY(DOWNARROW, "#str_07024"),
	NAMEKEY(PGDN, "#str_07050"),
	NAMEKEY(INS, "#str_07048"),
	NAMEKEY(DEL, "#str_07049"),
	NAMEKEY(LWIN, "#str_07030"),
	NAMEKEY(RWIN, "#str_07031"),
	NAMEKEY(APPS, "#str_07032"),
	NAMEKEY2(POWER),
	NAMEKEY2(SLEEP),

	// --

	NAMEKEY(MOUSE1, "#str_07054"),
	NAMEKEY(MOUSE2, "#str_07055"),
	NAMEKEY(MOUSE3, "#str_07056"),
	NAMEKEY(MOUSE4, "#str_07057"),
	NAMEKEY(MOUSE5, "#str_07058"),
	NAMEKEY(MOUSE6, "#str_07059"),
	NAMEKEY(MOUSE7, "#str_07060"),
	NAMEKEY(MOUSE8, "#str_07061"),

	NAMEKEY(MWHEELDOWN, "#str_07132"),
	NAMEKEY(MWHEELUP, "#str_07131"),

	//------------------------
	// Aliases to make it easier to bind or to support old configs
	//------------------------
	ALIASKEY("ALT", LALT),
	ALIASKEY("RIGHTALT", RALT),
	ALIASKEY("CTRL", LCTRL),
	ALIASKEY("SHIFT", LSHIFT),
	ALIASKEY("MENU", APPS),
	ALIASKEY("COMMAND", LALT),

	ALIASKEY("KP_HOME", KP_7),
	ALIASKEY("KP_UPARROW", KP_8),
	ALIASKEY("KP_PGUP", KP_9),
	ALIASKEY("KP_LEFTARROW", KP_4),
	ALIASKEY("KP_RIGHTARROW", KP_6),
	ALIASKEY("KP_END", KP_1),
	ALIASKEY("KP_DOWNARROW", KP_2),
	ALIASKEY("KP_PGDN", KP_3),
	ALIASKEY("KP_INS", KP_0),
	ALIASKEY("KP_DEL", KP_DOT),
	ALIASKEY("KP_NUMLOCK", NUMLOCK),

	ALIASKEY("-", MINUS),
	ALIASKEY("=", EQUALS),
	ALIASKEY("[", LBRACKET),
	ALIASKEY("]", RBRACKET),
	ALIASKEY("\\", BACKSLASH),
	ALIASKEY("/", SLASH),
	ALIASKEY(",", COMMA),
	ALIASKEY(".", PERIOD),

	{K_NONE, NULL, NULL}
};*/

class idKey {
public:
	idKey() { down = false; repeats = 0; usercmdAction = 0; }
	bool			down;
	int				repeats;		// if > 1, it is autorepeating
	std::string		binding;
	int				usercmdAction;	// for testing by the asyncronous usercmd generation
};

std::vector<idKey> keys;

/*
========================
idKeyInput::StringToKeyNum
========================
*/
keyNum_t idKeyInput::StringToKeyNum(const std::string& str) {

	if (str.empty()) {
		return keyNum_t::K_NONE;
	}

	// scan for a text match
	auto iter = std::find_if(keynames.begin(), keynames.end(), [&str](auto& kn) {return kn.name == str; });

	if (iter != keynames.end())
		return iter->keynum;

	return keyNum_t::K_NONE;
}

/*
========================
idKeyInput::KeyNumToString
========================
*/
const std::string idKeyInput::KeyNumToString(keyNum_t keynum) {
	// check for a key string
	auto iter = std::find_if(keynames.begin(), keynames.end(), [&keynum](auto& kn) {return kn.keynum == keynum; });

	if (iter != keynames.end())
		return iter->name;

	return "?";
}

/*
===================
idKeyInput::SetBinding
===================
*/
void idKeyInput::SetBinding(int keynum, const std::string& binding) {
	if (keynum == -1) {
		return;
	}

	// Clear out all button states so we aren't stuck forever thinking this key is held down
	usercmdGen->Clear();

	// allocate memory for new binding
	keys[keynum].binding = binding;

	// find the action for the async command generation
	keys[keynum].usercmdAction = usercmdGen->CommandStringUsercmdData(binding);

	// consider this like modifying an archived cvar, so the
	// file write will be triggered at the next oportunity
	//cvarSystem->SetModifiedFlags(CVAR_ARCHIVE);
}

/*
===================
Key_Bind_f
===================
*/
void Key_Bind_f(const idCmdArgs& args) {
	int			i, c, b;
	std::string cmd;
	cmd.reserve(MAX_STRING_CHARS);

	c = args.Argc();

	if (c < 2) {
		common->Printf("bind <key> [command] : attach a command to a key\n");
		return;
	}
	b = static_cast<int>(idKeyInput::StringToKeyNum(args.Argv(1)));
	if (b == static_cast<int>(keyNum_t::K_NONE)) {
		common->Printf("\"%s\" isn't a valid key\n", args.Argv(1).c_str());
		return;
	}

	if (c == 2) {
		if (!keys[b].binding.empty()) {
			common->Printf("\"%s\" = \"%s\"\n", args.Argv(1).c_str(), keys[b].binding.c_str());
		}
		else {
			common->Printf("\"%s\" is not bound\n", args.Argv(1).c_str());
		}
		return;
	}

	// copy the rest of the command line
	cmd[0] = 0;		// start out with a null string
	for (i = 2; i < c; i++) {
		cmd.append(args.Argv(i));
		if (i != (c - 1)) {
			cmd.append(" ");
		}
	}

	idKeyInput::SetBinding(b, cmd);
}

/*
============
Key_ListBinds_f
============
*/
void Key_ListBinds_f(const idCmdArgs& args) {
	for (int i = 0; i < static_cast<int>(keyNum_t::K_LAST_KEY); i++) {
		if (!keys[i].binding.empty()) {
			common->Printf("%s \"%s\"\n", idKeyInput::KeyNumToString((keyNum_t)i).c_str(), keys[i].binding.c_str());
		}
	}
}

/*
===================
idKeyInput::PreliminaryKeyEvent

Tracks global key up/down state
Called by the system for both key up and key down events
===================
*/
void idKeyInput::PreliminaryKeyEvent(int keynum, bool down) {
	keys[keynum].down = down;
}

/*
=================
idKeyInput::ExecKeyBinding
=================
*/
bool idKeyInput::ExecKeyBinding(int keynum) {
	// commands that are used by the async thread
	// don't add text
	if (keys[keynum].usercmdAction) {
		return false;
	}

	// send the bound action
	if (!keys[keynum].binding.empty()) {
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, keys[keynum].binding.c_str());
		cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");
	}
	return true;
}

/*
===================
idKeyInput::Init
===================
*/
void idKeyInput::Init() {

	keys.resize(static_cast<int>(keyNum_t::K_LAST_KEY));

	// register our functions
	cmdSystem->AddCommand("bind", Key_Bind_f, CMD_FL_SYSTEM, "binds a command to a key");
	cmdSystem->AddCommand("listBinds", Key_ListBinds_f, CMD_FL_SYSTEM, "lists key bindings");
}

/*
===================
idKeyInput::Shutdown
===================
*/
void idKeyInput::Shutdown() {
	keys.clear();
}

/*
===================
idKeyInput::IsDown
===================
*/
bool idKeyInput::IsDown(int keynum) {
	if (keynum == -1) {
		return false;
	}

	return keys[keynum].down;
}

/*
===================
idKeyInput::GetUsercmdAction
===================
*/
int idKeyInput::GetUsercmdAction(int keynum) {
	return keys[keynum].usercmdAction;
}