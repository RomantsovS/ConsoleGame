#ifndef IDLIB_TEXT_CMDARGS_H_
#define IDLIB_TEXT_CMDARGS_H_

#include <string>
#include <vector>

#include "sys/sys_types.h"

class idCmdArgs {
public:
	idCmdArgs() { }
	idCmdArgs(const std::string &text, bool keepAsStrings) { TokenizeString(text, keepAsStrings); }

	void operator=(const idCmdArgs& args);

	// The functions that execute commands get their parameters with these functions.
	int Argc() const { return argv.size(); }
	// Argv() will return an empty string, not NULL if arg >= argc.
	const std::string Argv(size_t arg) const { return (arg >= 0 && arg < argv.size()) ? argv[arg] : std::string(); }
	// Returns a single string containing argv(start) to argv(end)
	// escapeArgs is a fugly way to put the string back into a state ready to tokenize again
	const std::string Args(size_t start = 1, size_t end = -1, bool escapeArgs = false) const;

	// Takes a null terminated string and breaks the string up into arg tokens.
	// Does not need to be /n terminated.
	// Set keepAsStrings to true to only seperate tokens from whitespace and comments, ignoring punctuation
	void TokenizeString(const std::string &text, bool keepAsStrings);

	//void AppendArg(std::string &text);
	void Clear() { argv.clear(); }
	//const std::vector<std::string>& GetArgs();

private:
	std::string CutQuotes(const std::string& str);
	static const int		MAX_COMMAND_ARGS = 64;
	static const int		MAX_COMMAND_STRING = 2 * MAX_STRING_CHARS;

	//int argc; // number of arguments
	std::vector<std::string> argv;			// points into tokenized
	//std::string tokenized;		// will have 0 bytes inserted
};

#endif
