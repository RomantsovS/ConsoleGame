#include "idlib/precompiled.h"
#pragma hdrstop

idCVar net_allowCheats("net_allowCheats", "1", CVAR_BOOL | CVAR_NOCHEAT, "Allow cheats in multiplayer");

struct commandDef_t {
	commandDef_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		common->DPrintf("%s ctor\n", "commandDef_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}

	~commandDef_t() {
#ifdef DEBUG_PRINT_Ctor_Dtor
		if(isCommonExists)
			common->DPrintf("%s dtor\n", "commandDef_t");
#endif // DEBUG_PRINT_Ctor_Dtor
	}
	commandDef_t(const commandDef_t&) = default;
	commandDef_t& operator=(const commandDef_t&) = default;
	commandDef_t(commandDef_t&&) = default;
	commandDef_t& operator=(commandDef_t&&) = default;

	std::shared_ptr<commandDef_t> next;
	std::string name;
	cmdFunction_t function;
	//argCompletion_t			argCompletion;
	int flags;
	std::string description;
};

/*
================================================
idCmdSystemLocal
================================================
*/
class idCmdSystemLocal : public idCmdSystem {
public:
	void			Init() override;
	void			Shutdown() noexcept override;

	void AddCommand(const std::string& cmdName, cmdFunction_t function, int flags,
		const std::string& description/*, argCompletion_t argCompletion = NULL*/) override;
	void RemoveFlaggedCommands(int flags) override;

	//virtual void			ExecuteCommandText(const char* text);
	void			AppendCommandText(const std::string& text) override;

	void			BufferCommandText(cmdExecution_t exec, const std::string &text) override;
	void			ExecuteCommandBuffer() override;

	std::list<std::shared_ptr<commandDef_t>>& GetCommands() noexcept { return commands; }

private:
	static const int		MAX_CMD_BUFFER = 0x10000;
	
	std::list<std::shared_ptr<commandDef_t>> commands;

	int						textLength;
	unsigned char			textBuf[MAX_CMD_BUFFER];

private:
	void					ExecuteTokenizedString(const idCmdArgs& args);
	void					InsertCommandText(const std::string &text);

	static void				ListByFlags(const idCmdArgs& args, cmdFlags_t flags);
	static void				List_f(const idCmdArgs& args);
	static void				SystemList_f(const idCmdArgs& args);
	/*static void			RendererList_f(const idCmdArgs& args);
	static void				SoundList_f(const idCmdArgs& args);
	static void				GameList_f(const idCmdArgs& args);
	static void				ToolList_f(const idCmdArgs& args);*/
	static void				Exec_f(const idCmdArgs& args);
	/*static void				Vstr_f(const idCmdArgs& args);
	static void				Echo_f(const idCmdArgs& args);
	static void				Parse_f(const idCmdArgs& args);
	static void				Wait_f(const idCmdArgs& args);
	static void				PrintMemInfo_f(const idCmdArgs& args);*/

};

idCmdSystemLocal cmdSystemLocal;
idCmdSystem* cmdSystem = &cmdSystemLocal;

/*
============
idCmdSystemLocal::ListByFlags
============
*/
void idCmdSystemLocal::ListByFlags(const idCmdArgs& args, cmdFlags_t flags) {
	//idStr match;
	std::vector<std::shared_ptr<commandDef_t>> cmd_list(cmdSystemLocal.GetCommands().size());

	/*if (args.Argc() > 1) {
		match = args.Args(1, -1);
		match.Replace(" ", "");
	}
	else {
		match = "";
	}*/

	/*for (auto cmd = cmdSystemLocal.GetCommands(); cmd; cmd = cmd->next) {
		if (!(cmd->flags & flags)) {
			continue;
		}
		if (match.Length() && idStr(cmd->name).Filter(match, false) == 0) {
			continue;
		}

		cmdList.push_back(cmd);
	}*/

	std::copy_if(cmdSystemLocal.GetCommands().begin(), cmdSystemLocal.GetCommands().end(), cmd_list.begin(), [&](auto& cmd) noexcept {return cmd->flags& flags; });

	//cmdList.SortWithTemplate( idSort_CommandDef() );

	for (auto cmd : cmd_list) {
		common->Printf("  %-21s %s\n", cmd->name.c_str(), cmd->description.c_str());
	}

	common->Printf("%i commands\n", cmd_list.size());
}

/*
============
idCmdSystemLocal::List_f
============
*/
void idCmdSystemLocal::List_f(const idCmdArgs& args) {
	idCmdSystemLocal::ListByFlags(args, CMD_FL_ALL);
}

/*
============
idCmdSystemLocal::SystemList_f
============
*/
void idCmdSystemLocal::SystemList_f(const idCmdArgs& args) {
	idCmdSystemLocal::ListByFlags(args, CMD_FL_SYSTEM);
}

/*
===============
idCmdSystemLocal::Exec_f
===============
*/
void idCmdSystemLocal::Exec_f(const idCmdArgs& args) {
	std::unique_ptr<char[]> f;
	int len;
	std::string filename;

	if (args.Argc() != 2) {
		common->Printf("exec <filename> : execute a script file\n");
		return;
	}

	filename = args.Argv(1);
	//filename.DefaultFileExtension(".cfg");
	f = fileSystem->ReadFile(filename, len, nullptr, true);
	if (len <= 0) {
		common->Printf("couldn't exec %s\n", args.Argv(1).c_str());
		return;
	}
	common->Printf("execing %s\n", args.Argv(1).c_str());

	cmdSystemLocal.BufferCommandText(CMD_EXEC_INSERT, f.get());
}

/*
============
idCmdSystemLocal::Init
============
*/
void idCmdSystemLocal::Init() {

	AddCommand("listCmds", List_f, CMD_FL_SYSTEM, "lists commands");
	AddCommand("listSystemCmds", SystemList_f, CMD_FL_SYSTEM, "lists system commands");
	/*AddCommand("listRendererCmds", RendererList_f, CMD_FL_SYSTEM, "lists renderer commands");
	AddCommand("listSoundCmds", SoundList_f, CMD_FL_SYSTEM, "lists sound commands");
	AddCommand("listGameCmds", GameList_f, CMD_FL_SYSTEM, "lists game commands");
	AddCommand("listToolCmds", ToolList_f, CMD_FL_SYSTEM, "lists tool commands");*/
	AddCommand("exec", Exec_f, CMD_FL_SYSTEM, "executes a config file"/*, ArgCompletion_ConfigName*/);
	/*AddCommand("vstr", Vstr_f, CMD_FL_SYSTEM, "inserts the current value of a cvar as command text");
	AddCommand("echo", Echo_f, CMD_FL_SYSTEM, "prints text");
	AddCommand("parse", Parse_f, CMD_FL_SYSTEM, "prints tokenized string");
	AddCommand("wait", Wait_f, CMD_FL_SYSTEM, "delays remaining buffered commands one or more frames");

	// link in all the commands declared with static idCommandLink variables or CONSOLE_COMMAND macros
	for (idCommandLink* link = CommandLinks(); link != NULL; link = link->next) {
		AddCommand(link->cmdName_, link->function_, CMD_FL_SYSTEM, link->description_, link->argCompletion_);
	}*/

	//completionString = "*";

	textLength = 0;
}

/*
============
idCmdSystemLocal::Shutdown
============
*/
void idCmdSystemLocal::Shutdown() noexcept {
	commands.clear();

	/*completionString.Clear();
	completionParms.Clear();
	tokenizedCmds.Clear();
	postReload.Clear();*/
}

/*
============
idCmdSystemLocal::AddCommand
============
*/
void idCmdSystemLocal::AddCommand(const std::string& cmdName, cmdFunction_t function, int flags,
	const std::string& description/*, argCompletion_t argCompletion = NULL*/) {

	// fail if the command already exists
	auto iter = std::find_if(commands.begin(), commands.end(), [&](auto cmd) noexcept {return cmd->name == cmdName; });
	
	if (iter != commands.end()) {
		if (function != (*iter)->function) {
			common->Printf("idCmdSystemLocal::AddCommand: %s already defined\n", cmdName);
		}
		return;
	}

	auto cmd = std::make_shared<commandDef_t>();

	cmd->name = cmdName;
	cmd->function = function;
	//cmd->argCompletion = argCompletion;
	cmd->flags = flags;
	cmd->description = description;
	commands.push_front(cmd);
}

/*
============
idCmdSystemLocal::RemoveFlaggedCommands
============
*/
void idCmdSystemLocal::RemoveFlaggedCommands(int flags) {
	commands.erase(std::remove_if(commands.begin(), commands.end(), [&](auto cmd) noexcept {return cmd->flags & flags; }), commands.end());
}

/*
============
idCmdSystemLocal::ExecuteTokenizedString
============
*/
void idCmdSystemLocal::ExecuteTokenizedString(const idCmdArgs &args) {
	// execute the command line
	if (!args.Argc()) {
		return;		// no tokens
	}

	// check registered command functions	
	for (auto iter = commands.begin(); iter != commands.end(); ++iter) {
		auto cmd = *iter;

		if (idStr::caseInSensStringCompareCpp11(args.Argv(0), cmd->name)) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			std::iter_swap(commands.begin(), iter);

			if ((cmd->flags & (CMD_FL_CHEAT | CMD_FL_TOOL))  && !net_allowCheats.GetBool()) {
				common->Printf("Command '%s' not valid in multiplayer mode.\n", cmd->name.c_str());
				return;
			}
			// perform the action
			if (!cmd->function) {
				break;
			}
			else {
				cmd->function(args);
			}
			return;
		}
	}

	// check cvars
	if (cvarSystem->Command(args)) {
		return;
	}

	common->Printf("Unknown command '%s'\n", args.Argv(0).c_str());
}

/*
============
idCmdSystemLocal::InsertCommandText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void idCmdSystemLocal::InsertCommandText(const std::string &text) {
	int		len;
	int		i;

	len = text.size() + 1;
	if (len + textLength > (int)sizeof(textBuf)) {
		common->Printf("idCmdSystemLocal::InsertText: buffer overflow\n");
		return;
	}

	// move the existing command text
	for (i = textLength - 1; i >= 0; i--) {
		textBuf[i + len] = textBuf[i];
	}

	// copy the new text in
	memcpy(textBuf, text.c_str(), len - 1);

	// add a \n
	textBuf[len - 1] = '\n';

	textLength += len;
}

/*
============
idCmdSystemLocal::AppendCommandText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void idCmdSystemLocal::AppendCommandText(const std::string& text) {
	int l;

	l = text.size();

	if (textLength + l >= (int)sizeof(textBuf)) {
		common->Printf("idCmdSystemLocal::AppendText: buffer overflow\n");
		return;
	}
	memcpy(textBuf + textLength, text.c_str(), l);
	textLength += l;
}

/*
============
idCmdSystemLocal::BufferCommandText
============
*/
void idCmdSystemLocal::BufferCommandText(cmdExecution_t exec, const std::string &text) {
	switch (exec) {
	/*case CMD_EXEC_NOW: {
		ExecuteCommandText(text);
		break;
	}*/
	case CMD_EXEC_INSERT: {
		InsertCommandText(text);
		break;
	}
	case CMD_EXEC_APPEND: {
		AppendCommandText(text);
		break;
	}
	default: {
		common->FatalError("idCmdSystemLocal::BufferCommandText: bad exec type");
	}
	}
}

/*
============
idCmdSystemLocal::ExecuteCommandBuffer
============
*/
void idCmdSystemLocal::ExecuteCommandBuffer() {
	int i;
	int quotes;

	while (textLength) {

		/*if (wait) {
			// skip out while text still remains in buffer, leaving it for next frame
			wait--;
			break;
		}*/

		// find a \n or ; line break
		gsl::not_null<gsl::zstring> text{ (char*)textBuf };

		quotes = 0;
		for (i = 0; i < textLength; i++) {
			if (text.operator->()[i] == '"') {
				quotes++;
			}
			if (!(quotes & 1) && text.operator->()[i] == ';') {
				break;	// don't break if inside a quoted string
			}
			if (text.operator->()[i] == '\n' || text.operator->()[i] == '\r') {
				break;
			}
		}

		text.operator->()[i] = 0;

		idCmdArgs args;

		/*if (!strcmp(text, "_execTokenized")) {
			args = tokenizedCmds[0];
			tokenizedCmds.RemoveIndex(0);
		}
		else {*/
			args.TokenizeString(text.operator->(), false);
		//}

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if (i == textLength) {
			textLength = 0;
		}
		else {
			i++;
			textLength -= i;
			memmove(text.operator->(), text.operator->() + i, textLength);
		}

		// execute the command line that we have already tokenized
		ExecuteTokenizedString(args);
	}
}