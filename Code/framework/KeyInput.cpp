#include "idlib/precompiled.h"
#pragma hdrstop

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
	keyname_t({keyNum_t::K_ESCAPE, "ESCAPE", "ESCAPE"}),
	keyname_t({keyNum_t::K_SPACE, "SPACE", "#str_07021"}),
	keyname_t({keyNum_t::K_TAB, "TAB", "#str_07018"}),
	keyname_t({keyNum_t::K_F1, "F1", "#str_07018"}),
	keyname_t({keyNum_t::K_UPARROW, "UPARROW", "#str_07023"}),
	keyname_t({keyNum_t::K_LEFTARROW, "LEFTARROW", "#str_07025"}),
	keyname_t({keyNum_t::K_RIGHTARROW, "RIGHTARROW", "#str_07026"}),
	keyname_t({keyNum_t::K_DOWNARROW, "DOWNARROW", "#str_07024"}),
	keyname_t({keyNum_t::K_ENTER, "ENTER", "#str_07019"}),
	keyname_t({keyNum_t::K_LSHIFT, "LSHIFT", "#str_07019"})
};

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
	auto iter = std::find_if(keynames.begin(), keynames.end(), [&str](auto& kn) noexcept {return kn.name == str; });

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
	auto iter = std::find_if(keynames.begin(), keynames.end(), [&keynum](auto& kn) noexcept {return kn.keynum == keynum; });

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
	cmd.reserve(max_string_chars);

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
void idKeyInput::PreliminaryKeyEvent(int keynum, bool down) noexcept {
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
void idKeyInput::Shutdown() noexcept {
	keys.clear();
}

/*
===================
idKeyInput::IsDown
===================
*/
bool idKeyInput::IsDown(int keynum) noexcept {
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
int idKeyInput::GetUsercmdAction(int keynum) noexcept {
	return keys[keynum].usercmdAction;
}