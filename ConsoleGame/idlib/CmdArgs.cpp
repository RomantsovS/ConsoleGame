#include "precompiled.h"
#pragma hdrstop

/*
============
idCmdArgs::operator=
============
*/
void idCmdArgs::operator=(const idCmdArgs& args) {
	//tokenized = args.tokenized;
	argv = args.argv;
}

/*
============
idCmdArgs::Args
============
*/
const std::string idCmdArgs::Args(size_t start, size_t end, bool escapeArgs) const {
	static char cmd_args[MAX_COMMAND_STRING];

	//assert(argc < MAX_COMMAND_ARGS);
	if (end <= 0) {
		end = argv.size() - 1;
	}
	else if (end >= argv.size()) {
		end = argv.size() - 1;
	}
	cmd_args[0] = '\0';
	if (escapeArgs) {
		strcat_s(cmd_args, "\"");
	}
	for (auto i = start; i <= end; i++) {
		if (i > start) {
			if (escapeArgs) {
				strcat_s(cmd_args, "\" \"");
			}
			else {
				strcat_s(cmd_args, " ");
			}
		}
		if (escapeArgs && argv[i].find_first_of('\\') != std::string::npos) {
			auto p = argv[i].begin();
			while (p != argv[i].end()) {
				if (*p == '\\') {
					strcat_s(cmd_args, "\\\\");
				}
				else {
					int l = strlen(cmd_args);
					cmd_args[l] = *p;
					cmd_args[l + 1] = '\0';
				}
				p++;
			}
		}
		else {
			strcat_s(cmd_args, argv[i].c_str());
		}
	}
	if (escapeArgs) {
		strcat_s(cmd_args, "\"");
	}

	return cmd_args;
}

/*
============
idCmdArgs::TokenizeString

Parses the given string into command line tokens.
The text is copied to a separate buffer and 0 characters
are inserted in the appropriate place. The argv array
will point into this temporary buffer.
============
*/
void idCmdArgs::TokenizeString(const std::string &text, bool keepAsStrings) {
	// clear previous args
	argv.clear();

	if (text.empty() || text.substr(0, 2) == "//") {
		return;
	}

	size_t found = 0, cur_found = 0;

	while (1) {
		if (argv.size() == MAX_COMMAND_ARGS) {
			return;			// this is usually something malicious
		}

		cur_found = text.find(' ', found);

		if (cur_found == std::string::npos) {
			argv.emplace_back(CutQuotes(text.substr(found)));

			return;
		}
		else if (cur_found == found) {
			++found;
		}
		else {
			auto str = text.substr(found, cur_found - found);

			if (str.substr(0, 2) == "//")
				return;

			argv.emplace_back(CutQuotes(str));

			found = cur_found + 1;
		}
	}
}

std::string idCmdArgs::CutQuotes(const std::string& str)
{
	std::string res(str);

	if (res[0] == '\"')
		res = res.substr(1);

	if (res[res.size() - 1] == '\"')
		res = res.substr(0, res.size() - 1);

	auto pos = res.find_last_of('\"');
	if (pos != std::string::npos) {
		res = res.substr(0, pos);
	}
	return res;
}
