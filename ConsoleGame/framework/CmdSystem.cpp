#include <memory>
#include <vector>
#include "../idlib/Str.h"

#include "CmdSystem.h"
#include "Common_local.h"
#include "FileSystem.h"

struct commandDef_t {
	std::shared_ptr<commandDef_t> next;
	std::string name;
	cmdFunction_t			function;
	//argCompletion_t			argCompletion;
	int						flags;
	std::string description;
};

/*
================================================
idCmdSystemLocal
================================================
*/
class idCmdSystemLocal : public idCmdSystem {
public:
	virtual void			Init() override;
	virtual void			Shutdown() override;

	virtual void AddCommand(const std::string& cmdName, cmdFunction_t function, int flags,
		const std::string& description/*, argCompletion_t argCompletion = NULL*/) override;
	virtual void RemoveFlaggedCommands(int flags);

	//virtual void			ExecuteCommandText(const char* text);
	virtual void			AppendCommandText(const std::string& text) override;

	virtual void			BufferCommandText(cmdExecution_t exec, const std::string &text) override;
	virtual void			ExecuteCommandBuffer() override;

	std::shared_ptr<commandDef_t> GetCommands() const { return commands; }

private:
	static const int		MAX_CMD_BUFFER = 0x10000;
	
	std::shared_ptr<commandDef_t> commands;

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
	std::vector<std::shared_ptr<commandDef_t>> cmdList;

	/*if (args.Argc() > 1) {
		match = args.Args(1, -1);
		match.Replace(" ", "");
	}
	else {
		match = "";
	}*/

	for (auto cmd = cmdSystemLocal.GetCommands(); cmd; cmd = cmd->next) {
		if (!(cmd->flags & flags)) {
			continue;
		}
		/*if (match.Length() && idStr(cmd->name).Filter(match, false) == 0) {
			continue;
		}*/

		cmdList.push_back(cmd);
	}

	//cmdList.SortWithTemplate( idSort_CommandDef() );

	for (size_t i = 0; i < cmdList.size(); i++) {
		auto cmd = cmdList[i];

		common->Printf("  %-21s %s\n", cmd->name, cmd->description);
	}

	common->Printf("%i commands\n", cmdList.size());
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
	char* f;
	int		len;
	std::string filename;

	if (args.Argc() != 2) {
		common->Printf("exec <filename> : execute a script file\n");
		return;
	}

	filename = args.Argv(1);
	//filename.DefaultFileExtension(".cfg");
	len = fileSystem->ReadFile(filename, reinterpret_cast<void**>(&f));
	if (!f) {
		common->Printf("couldn't exec %s\n", args.Argv(1).c_str());
		return;
	}
	common->Printf("execing %s\n", args.Argv(1).c_str());

	cmdSystemLocal.BufferCommandText(CMD_EXEC_INSERT, f);

	fileSystem->FreeFile(f);
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
void idCmdSystemLocal::Shutdown() {
	for (auto cmd = commands; cmd; cmd = commands) {
		commands = commands->next;
		cmd = nullptr;
	}

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
	for (auto cmd = commands; cmd; cmd = cmd->next) {
		if (cmdName == cmd->name) {
			if (function != cmd->function) {
				common->Printf("idCmdSystemLocal::AddCommand: %s already defined\n", cmdName);
			}
			return;
		}
	}

	auto cmd = std::make_shared<commandDef_t>();
	cmd->name = cmdName;
	cmd->function = function;
	//cmd->argCompletion = argCompletion;
	cmd->flags = flags;
	cmd->description = description;
	cmd->next = commands;
	commands = cmd;
}

/*
============
idCmdSystemLocal::RemoveFlaggedCommands
============
*/
void idCmdSystemLocal::RemoveFlaggedCommands(int flags) {
	decltype(commands) last, cmd;

	for (last = commands, cmd = last; cmd; cmd = last) {
		if (last->flags & flags) {
			last = last->next;
			cmd = nullptr;
			continue;
		}
		last = cmd->next;
	}
}

/*
============
idCmdSystemLocal::ExecuteTokenizedString
============
*/
void idCmdSystemLocal::ExecuteTokenizedString(const idCmdArgs &args) {
	std::shared_ptr<commandDef_t> cmd;

	// execute the command line
	if (!args.Argc()) {
		return;		// no tokens
	}

	// check registered command functions	
	for (auto prev = &commands; *prev; prev = &cmd->next) {
		cmd = *prev;
		if (idStr::caseInSensStringCompareCpp11(args.Argv(0), cmd->name)) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			*prev = cmd->next;
			cmd->next = commands;
			commands = cmd;

			if ((cmd->flags & (CMD_FL_CHEAT | CMD_FL_TOOL)) /*&& common->IsMultiplayer() && !net_allowCheats.GetBool()*/) {
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
	/*if (cvarSystem->Command(args)) {
		return;
	}*/

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
	int			i;
	char* text;
	int			quotes;
	idCmdArgs	args;

	while (textLength) {

		/*if (wait) {
			// skip out while text still remains in buffer, leaving it for next frame
			wait--;
			break;
		}*/

		// find a \n or ; line break
		text = (char*)textBuf;

		quotes = 0;
		for (i = 0; i < textLength; i++) {
			if (text[i] == '"') {
				quotes++;
			}
			if (!(quotes & 1) && text[i] == ';') {
				break;	// don't break if inside a quoted string
			}
			if (text[i] == '\n' || text[i] == '\r') {
				break;
			}
		}

		text[i] = 0;

		/*if (!strcmp(text, "_execTokenized")) {
			args = tokenizedCmds[0];
			tokenizedCmds.RemoveIndex(0);
		}
		else {*/
			args.TokenizeString(text, false);
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
			memmove(text, text + i, textLength);
		}

		// execute the command line that we have already tokenized
		ExecuteTokenizedString(args);
	}
}