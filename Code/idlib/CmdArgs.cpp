#include "precompiled.h"

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
	idLexer		lex;
	idToken		token, number;
	int			len, totalLen;

	// clear previous args
	argv.clear();

	if (text.empty()) {
		return;
	}

	lex.LoadMemory(text.c_str(), text.size(), "idCmdSystemLocal::TokenizeString");
	/*lex.SetFlags(LEXFL_NOERRORS
		| LEXFL_NOWARNINGS
		| LEXFL_NOSTRINGCONCAT
		| LEXFL_ALLOWPATHNAMES
		| LEXFL_NOSTRINGESCAPECHARS
		| LEXFL_ALLOWIPADDRESSES | (keepAsStrings ? LEXFL_ONLYSTRINGS : 0));
		*/
	totalLen = 0;

	while (1) {
		if (argv.size() == MAX_COMMAND_ARGS) {
			return;			// this is usually something malicious
		}

		if (!lex.ReadToken(&token)) {
			return;
		}

		// check for negative numbers
		if (!keepAsStrings && (token == "-")) {
			if (lex.CheckTokenType(TT_NUMBER, 0, &number)) {
				token.assign("-" + number);
			}
		}

		// check for cvar expansion
		if (token == "$") {
			if (!lex.ReadToken(&token)) {
				return;
			}
			/*if (idLib::cvarSystem) {
				token = idLib::cvarSystem->GetCVarString(token.c_str());
			}*/
			else {
				token.assign("<unknown>");
			}
		}

		len = token.size();

		/*if (totalLen + len + 1 > sizeof(tokenized)) {
			return;			// this is usually something malicious
		}

		// regular token
		argv[argc] = tokenized + totalLen;
		argc++;*/
		argv.push_back(token);

		//idStr::Copynz(tokenized + totalLen, token.c_str(), sizeof(tokenized) - totalLen);

		totalLen += len + 1;
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
