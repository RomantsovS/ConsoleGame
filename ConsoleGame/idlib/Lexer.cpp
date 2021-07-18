#include "precompiled.h"
#pragma hdrstop

#define PUNCTABLE

//longer punctuations first
std::vector<punctuation_t> default_punctuations {
	//binary operators
	{">>=",P_RSHIFT_ASSIGN},
	{"<<=",P_LSHIFT_ASSIGN},
	//
	{"...",P_PARMS},
	//define merge operator
	{"##",P_PRECOMPMERGE},				// pre-compiler
	//logic operators
	{"&&",P_LOGIC_AND},					// pre-compiler
	{"||",P_LOGIC_OR},					// pre-compiler
	{">=",P_LOGIC_GEQ},					// pre-compiler
	{"<=",P_LOGIC_LEQ},					// pre-compiler
	{"==",P_LOGIC_EQ},					// pre-compiler
	{"!=",P_LOGIC_UNEQ},				// pre-compiler
	//arithmatic operators
	{"*=",P_MUL_ASSIGN},
	{"/=",P_DIV_ASSIGN},
	{"%=",P_MOD_ASSIGN},
	{"+=",P_ADD_ASSIGN},
	{"-=",P_SUB_ASSIGN},
	{"++",P_INC},
	{"--",P_DEC},
	//binary operators
	{"&=",P_BIN_AND_ASSIGN},
	{"|=",P_BIN_OR_ASSIGN},
	{"^=",P_BIN_XOR_ASSIGN},
	{">>",P_RSHIFT},					// pre-compiler
	{"<<",P_LSHIFT},					// pre-compiler
	//reference operators
	{"->",P_POINTERREF},
	//C++
	{"::",P_CPP1},
	{".*",P_CPP2},
	//arithmatic operators
	{"*",P_MUL},						// pre-compiler
	{"/",P_DIV},						// pre-compiler
	{"%",P_MOD},						// pre-compiler
	{"+",P_ADD},						// pre-compiler
	{"-",P_SUB},						// pre-compiler
	{"=",P_ASSIGN},
	//binary operators
	{"&",P_BIN_AND},					// pre-compiler
	{"|",P_BIN_OR},						// pre-compiler
	{"^",P_BIN_XOR},					// pre-compiler
	{"~",P_BIN_NOT},					// pre-compiler
	//logic operators
	{"!",P_LOGIC_NOT},					// pre-compiler
	{">",P_LOGIC_GREATER},				// pre-compiler
	{"<",P_LOGIC_LESS},					// pre-compiler
	//reference operator
	{".",P_REF},
	//seperators
	{",",P_COMMA},						// pre-compiler
	{";",P_SEMICOLON},
	//label indication
	{":",P_COLON},						// pre-compiler
	//if statement
	{"?",P_QUESTIONMARK},				// pre-compiler
	//embracements
	{"(",P_PARENTHESESOPEN},			// pre-compiler
	{")",P_PARENTHESESCLOSE},			// pre-compiler
	{"{",P_BRACEOPEN},					// pre-compiler
	{"}",P_BRACECLOSE},					// pre-compiler
	{"[",P_SQBRACKETOPEN},
	{"]",P_SQBRACKETCLOSE},
	//
	{"\\",P_BACKSLASH},
	//precompiler operator
	{"#",P_PRECOMP},					// pre-compiler
	{"$",P_DOLLAR},
	{NULL, 0}
};

std::vector<int> default_punctuationtable(256);
std::vector<int> default_nextpunctuation(default_punctuations.size());
int default_setup;

/*
================
idLexer::CreatePunctuationTable
================
*/
void idLexer::CreatePunctuationTable(const std::vector<punctuation_t>& punctuations) {
	int i, n, lastp;
	const punctuation_t* p;

	//get memory for the table
	if (&punctuations == &default_punctuations) {
		idLexer::punctuationtable = &default_punctuationtable;
		idLexer::nextpunctuation = &default_nextpunctuation;
		if (default_setup) {
			return;
		}
		default_setup = true;
		i = default_punctuations.size();
	}
	//memset(idLexer::punctuationtable, 0xFF, 256 * sizeof(int));
	std::fill((*idLexer::punctuationtable).begin(), (*idLexer::punctuationtable).end(), -1);
	//memset(idLexer::nextpunctuation, 0xFF, i * sizeof(int));
	std::fill((*idLexer::nextpunctuation).begin(), (*idLexer::nextpunctuation).end(), -1);
	//add the punctuations in the list to the punctuation table
	for (i = 0; punctuations[i].p; i++) {
		auto newp = &punctuations[i];
		lastp = -1;
		//sort the punctuations in this table entry on length (longer punctuations first)
		for (n = (*idLexer::punctuationtable)[(unsigned int)newp->p[0]]; n >= 0; n = (*idLexer::nextpunctuation)[n]) {
			p = &punctuations[n];
			if (strlen(p->p) < strlen(newp->p)) {
				(*idLexer::nextpunctuation)[i] = n;
				if (lastp >= 0) {
					(*idLexer::nextpunctuation)[lastp] = i;
				}
				else {
					(*idLexer::punctuationtable)[(unsigned int)newp->p[0]] = i;
				}
				break;
			}
			lastp = n;
		}
		if (n < 0) {
			(*idLexer::nextpunctuation)[i] = -1;
			if (lastp >= 0) {
				(*idLexer::nextpunctuation)[lastp] = i;
			}
			else {
				(*idLexer::punctuationtable)[(unsigned int)newp->p[0]] = i;
			}
		}
	}
}

/*
================
idLexer::Error
================
*/
void idLexer::Error(const char* str, ...) {
	char text[MAX_STRING_CHARS];
	va_list ap;

	/*hadError = true;

	if (idLexer::flags & LEXFL_NOERRORS) {
		return;
	}*/

	va_start(ap, str);
	vsprintf_s(text, str, ap);
	va_end(ap);

	if (false/*idLexer::flags & LEXFL_NOFATALERRORS*/) {
		common->Warning("file %s, line %d: %s", idLexer::filename.c_str(), idLexer::line, text);
	}
	else {
		common->Error("file %s, line %d: %s", idLexer::filename.c_str(), idLexer::line, text);
	}
}

/*
================
idLexer::Warning
================
*/
void idLexer::Warning(const char* str, ...) {
	char text[MAX_STRING_CHARS];
	va_list ap;

	/*if (idLexer::flags & LEXFL_NOWARNINGS) {
		return;
	}*/

	va_start(ap, str);
	vsprintf_s(text, str, ap);
	va_end(ap);
	common->Warning("file %s, line %d: %s", idLexer::filename.c_str(), idLexer::line, text);
}

/*
================
idLexer::SetPunctuations
================
*/
void idLexer::SetPunctuations(const std::vector<punctuation_t>* p) {
#ifdef PUNCTABLE
	if (p) {
		idLexer::CreatePunctuationTable(*p);
	}
	else {
		idLexer::CreatePunctuationTable(default_punctuations);
	}
#endif //PUNCTABLE
	if (p) {
		idLexer::punctuations = p;
	}
	else {
		idLexer::punctuations = &default_punctuations;
	}
}

/*
================
idLexer::ReadWhiteSpace

Reads spaces, tabs, C-like comments etc.
When a newline character is found the scripts line counter is increased.
================
*/
int idLexer::ReadWhiteSpace() {
	while (1) {
		// skip white space
		while (*idLexer::script_p <= ' ') {
			if (!*idLexer::script_p) {
				return 0;
			}
			if (*idLexer::script_p == '\n') {
				idLexer::line++;
			}
			idLexer::script_p++;
		}
		// skip comments
		if (*idLexer::script_p == '/') {
			// comments //
			if (*(idLexer::script_p + 1) == '/') {
				idLexer::script_p++;
				do {
					idLexer::script_p++;
					if (!*idLexer::script_p) {
						return 0;
					}
				} while (*idLexer::script_p != '\n');
				idLexer::line++;
				idLexer::script_p++;
				if (!*idLexer::script_p) {
					return 0;
				}
				continue;
			}
			// comments /* */
			else if (*(idLexer::script_p + 1) == '*') {
				idLexer::script_p++;
				while (1) {
					idLexer::script_p++;
					if (!*idLexer::script_p) {
						return 0;
					}
					if (*idLexer::script_p == '\n') {
						idLexer::line++;
					}
					else if (*idLexer::script_p == '/') {
						if (*(idLexer::script_p - 1) == '*') {
							break;
						}
						if (*(idLexer::script_p + 1) == '*') {
							idLexer::Warning("nested comment");
						}
					}
				}
				idLexer::script_p++;
				if (!*idLexer::script_p) {
					return 0;
				}
				idLexer::script_p++;
				if (!*idLexer::script_p) {
					return 0;
				}
				continue;
			}
		}
		break;
	}
	return 1;
}

/*
================
idLexer::ReadString

Escape characters are interpretted.
Reads two strings with only a white space between them as one string.
================
*/
int idLexer::ReadString(gsl::not_null<idToken*> token, int quote) {
	int tmpline;
	const char* tmpscript_p;
	//char ch;

	if (quote == '\"') {
		token->type = TT_STRING;
	}
	else {
		token->type = TT_LITERAL;
	}

	// leading quote
	idLexer::script_p++;

	while (1) {
		// if there is an escape character and escape characters are allowed
		/*if (*idLexer::script_p == '\\' && !(idLexer::flags & LEXFL_NOSTRINGESCAPECHARS)) {
			if (!idLexer::ReadEscapeCharacter(&ch)) {
				return 0;
			}
			token->AppendDirty(ch);
		}
		// if a trailing quote
		else*/ if (*idLexer::script_p == quote) {
			// step over the quote
			idLexer::script_p++;
			// if consecutive strings should not be concatenated
			/*if ((idLexer::flags & LEXFL_NOSTRINGCONCAT) &&
				(!(idLexer::flags & LEXFL_ALLOWBACKSLASHSTRINGCONCAT) || (quote != '\"'))) {
				break;
			}*/

			tmpscript_p = idLexer::script_p;
			tmpline = idLexer::line;
			// read white space between possible two consecutive strings
			if (!idLexer::ReadWhiteSpace()) {
				idLexer::script_p = tmpscript_p;
				idLexer::line = tmpline;
				break;
			}

			if (true/*idLexer::flags & LEXFL_NOSTRINGCONCAT*/) {
				if (*idLexer::script_p != '\\') {
					idLexer::script_p = tmpscript_p;
					idLexer::line = tmpline;
					break;
				}
				// step over the '\\'
				idLexer::script_p++;
				if (!idLexer::ReadWhiteSpace() || (*idLexer::script_p != quote)) {
					idLexer::Error("expecting string after '\' terminated line");
					return 0;
				}
			}

			// if there's no leading qoute
			if (*idLexer::script_p != quote) {
				idLexer::script_p = tmpscript_p;
				idLexer::line = tmpline;
				break;
			}
			// step over the new leading quote
			idLexer::script_p++;
		}
		else {
			if (*idLexer::script_p == '\0') {
				idLexer::Error("missing trailing quote");
				return 0;
			}
			if (*idLexer::script_p == '\n') {
				idLexer::Error("newline inside string");
				return 0;
			}
			token->AppendDirty(*idLexer::script_p++);
		}
	}
	//token->data[token->len] = '\0';

	if (token->type == TT_LITERAL) {
		/*if (!(idLexer::flags & LEXFL_ALLOWMULTICHARLITERALS)) {
			if (token->Length() != 1) {
				idLexer::Warning("literal is not one character long");
			}
		}*/
		token->subtype = (*token)[0];
	}
	else {
		// the sub type is the length of the string
		token->subtype = token->size();
	}
	return 1;
}

/*
================
idLexer::ReadName
================
*/
int idLexer::ReadName(gsl::not_null<idToken*> token) {
	char c;

	token->type = TT_NAME;
	do {
		token->AppendDirty(*idLexer::script_p++);
		c = *idLexer::script_p;
	} while ((c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') ||
		c == '_' ||
		// if treating all tokens as strings, don't parse '-' as a seperate token
		//((idLexer::flags & LEXFL_ONLYSTRINGS) && (c == '-')) ||
		// if special path name characters are allowed
		(/*(idLexer::flags & LEXFL_ALLOWPATHNAMES) &&*/ (c == '/' || c == '\\' || c == ':' || c == '.')));
	//token->data[token->len] = '\0';
	//the sub type is the length of the name
	token->subtype = token->size();
	return 1;
}

/*
================
idLexer::ReadNumber
================
*/
int idLexer::ReadNumber(gsl::not_null<idToken*> token) {
	int i;
	int dot;
	char c, c2;

	token->type = TT_NUMBER;
	token->subtype = 0;
	token->intvalue = 0;
	token->floatvalue = 0;

	c = *idLexer::script_p;
	c2 = *(idLexer::script_p + 1);

	if (c == '0' && c2 != '.') {
		// check for a hexadecimal number
		if (c2 == 'x' || c2 == 'X') {
			token->AppendDirty(*idLexer::script_p++);
			token->AppendDirty(*idLexer::script_p++);
			c = *idLexer::script_p;
			while ((c >= '0' && c <= '9') ||
				(c >= 'a' && c <= 'f') ||
				(c >= 'A' && c <= 'F')) {
				token->AppendDirty(c);
				c = *(++idLexer::script_p);
			}
			token->subtype = TT_HEX | TT_INTEGER;
		}
		// check for a binary number
		else if (c2 == 'b' || c2 == 'B') {
			token->AppendDirty(*idLexer::script_p++);
			token->AppendDirty(*idLexer::script_p++);
			c = *idLexer::script_p;
			while (c == '0' || c == '1') {
				token->AppendDirty(c);
				c = *(++idLexer::script_p);
			}
			token->subtype = TT_BINARY | TT_INTEGER;
		}
		// its an octal number
		else {
			token->AppendDirty(*idLexer::script_p++);
			c = *idLexer::script_p;
			while (c >= '0' && c <= '7') {
				token->AppendDirty(c);
				c = *(++idLexer::script_p);
			}
			token->subtype = TT_OCTAL | TT_INTEGER;
		}
	}
	else {
		// decimal integer or floating point number or ip address
		dot = 0;
		while (1) {
			if (c >= '0' && c <= '9') {
			}
			else if (c == '.') {
				dot++;
			}
			else {
				break;
			}
			token->AppendDirty(c);
			c = *(++idLexer::script_p);
		}
		if (c == 'e' && dot == 0) {
			//We have scientific notation without a decimal point
			dot++;
		}
		// if a floating point number
		if (dot == 1) {
			token->subtype = TT_DECIMAL | TT_FLOAT;
			// check for floating point exponent
			if (c == 'e') {
				//Append the e so that GetFloatValue code works
				token->AppendDirty(c);
				c = *(++idLexer::script_p);
				if (c == '-') {
					token->AppendDirty(c);
					c = *(++idLexer::script_p);
				}
				else if (c == '+') {
					token->AppendDirty(c);
					c = *(++idLexer::script_p);
				}
				while (c >= '0' && c <= '9') {
					token->AppendDirty(c);
					c = *(++idLexer::script_p);
				}
			}
			// check for floating point exception infinite 1.#INF or indefinite 1.#IND or NaN
			else if (c == '#') {
				c2 = 4;
				/*if (CheckString("INF")) {
					token->subtype |= TT_INFINITE;
				}
				else if (CheckString("IND")) {
					token->subtype |= TT_INDEFINITE;
				}
				else if (CheckString("NAN")) {
					token->subtype |= TT_NAN;
				}
				else if (CheckString("QNAN")) {
					token->subtype |= TT_NAN;
					c2++;
				}
				else if (CheckString("SNAN")) {
					token->subtype |= TT_NAN;
					c2++;
				}*/
				for (i = 0; i < c2; i++) {
					token->AppendDirty(c);
					c = *(++idLexer::script_p);
				}
				while (c >= '0' && c <= '9') {
					token->AppendDirty(c);
					c = *(++idLexer::script_p);
				}
				/*if (!(idLexer::flags & LEXFL_ALLOWFLOATEXCEPTIONS)) {
					token->AppendDirty(0);	// zero terminate for c_str
					idLexer::Error("parsed %s", token->c_str());
				}*/
			}
		}
		else if (dot > 1) {
			/*if (!(idLexer::flags & LEXFL_ALLOWIPADDRESSES)) {
				idLexer::Error("more than one dot in number");
				return 0;
			}*/
			if (dot != 3) {
				idLexer::Error("ip address should have three dots");
				return 0;
			}
			token->subtype = TT_IPADDRESS;
		}
		else {
			token->subtype = TT_DECIMAL | TT_INTEGER;
		}
	}

	if (token->subtype & TT_FLOAT) {
		if (c > ' ') {
			// single-precision: float
			if (c == 'f' || c == 'F') {
				token->subtype |= TT_SINGLE_PRECISION;
				idLexer::script_p++;
			}
			// extended-precision: long double
			else if (c == 'l' || c == 'L') {
				token->subtype |= TT_EXTENDED_PRECISION;
				idLexer::script_p++;
			}
			// default is double-precision: double
			else {
				token->subtype |= TT_DOUBLE_PRECISION;
			}
		}
		else {
			token->subtype |= TT_DOUBLE_PRECISION;
		}
	}
	else if (token->subtype & TT_INTEGER) {
		if (c > ' ') {
			// default: signed long
			for (i = 0; i < 2; i++) {
				// long integer
				if (c == 'l' || c == 'L') {
					token->subtype |= TT_LONG;
				}
				// unsigned integer
				else if (c == 'u' || c == 'U') {
					token->subtype |= TT_UNSIGNED;
				}
				else {
					break;
				}
				c = *(++idLexer::script_p);
			}
		}
	}
	else if (token->subtype & TT_IPADDRESS) {
		if (c == ':') {
			token->AppendDirty(c);
			c = *(++idLexer::script_p);
			while (c >= '0' && c <= '9') {
				token->AppendDirty(c);
				c = *(++idLexer::script_p);
			}
			token->subtype |= TT_IPPORT;
		}
	}
	//token->data[token->len] = '\0';
	return 1;
}

/*
================
idLexer::ReadPunctuation
================
*/
int idLexer::ReadPunctuation(gsl::not_null<idToken*> token) {
	int l, n, i;

#ifdef PUNCTABLE
	for (n = (*idLexer::punctuationtable)[(unsigned int)*(idLexer::script_p)]; n >= 0; n = (*idLexer::nextpunctuation)[n])
	{
		const punctuation_t punc = (*idLexer::punctuations)[n];
#else
	int i;

	for (i = 0; idLexer::punctuations[i].p; i++) {
		punc = &idLexer::punctuations[i];
#endif
		gsl::not_null<gsl::czstring<>> p{ punc.p };
		// check for this punctuation in the script
		for (l = 0; p.operator->()[l] && idLexer::script_p[l]; l++) {
			if (idLexer::script_p[l] != p.operator->()[l]) {
				break;
			}
		}
		if (!p.operator->()[l]) {
			//
			token->resize(token->size() + 1);
			for (i = 0; i < l; i++) {
				token->at(i) = p.operator->()[i];
			}

			//
			idLexer::script_p += l;
			token->type = TT_PUNCTUATION;
			// sub type is the punctuation id
			token->subtype = punc.n;
			return 1;
		}
	}
	return 0;
}

/*
================
idLexer::ReadToken
================
*/
int idLexer::ReadToken(gsl::not_null<idToken*> token) {
	int c;

	if (!loaded) {
		common->Error("idLexer::ReadToken: no file loaded");
		return 0;
	}

	if (script_p == NULL) {
		return 0;
	}

	// if there is a token available (from unreadToken)
	if (tokenavailable) {
		tokenavailable = 0;
		*token = idLexer::token;
		return 1;
	}
	// save script pointer
	lastScript_p = script_p;
	// save line counter
	lastline = line;
	// clear the token stuff
	token->clear();
	// start of the white space
	/*whiteSpaceStart_p = script_p;
	token->whiteSpaceStart_p = script_p;*/
	// read white space before token
	if (!ReadWhiteSpace()) {
		return 0;
	}
	// end of the white space
	/*idLexer::whiteSpaceEnd_p = script_p;
	token->whiteSpaceEnd_p = script_p;*/
	// line the token is on
	token->line = line;
	// number of lines crossed before token
	token->linesCrossed = line - lastline;
	// clear token flags
	//token->flags = 0;

	c = *idLexer::script_p;

	// if we're keeping everything as whitespace deliminated strings
	/*if (idLexer::flags & LEXFL_ONLYSTRINGS) {
		// if there is a leading quote
		if (c == '\"' || c == '\'') {
			if (!idLexer::ReadString(token, c)) {
				return 0;
			}
		}
		else if (!idLexer::ReadName(token)) {
			return 0;
		}
	}
	// if there is a number
	else*/ if ((c >= '0' && c <= '9') ||
		(c == '.' && (*(idLexer::script_p + 1) >= '0' && *(idLexer::script_p + 1) <= '9'))) {
		if (!idLexer::ReadNumber(token)) {
			return 0;
		}
		// if names are allowed to start with a number
		/*if (idLexer::flags & LEXFL_ALLOWNUMBERNAMES) {
			c = *idLexer::script_p;
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
				if (!idLexer::ReadName(token)) {
					return 0;
				}
			}
		}*/
	}
	// if there is a leading quote
	else if (c == '\"' || c == '\'') {
		if (!idLexer::ReadString(token, c)) {
			return 0;
		}
	}
	// if there is a name
	else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
		if (!idLexer::ReadName(token)) {
			return 0;
		}
	}
	// names may also start with a slash when pathnames are allowed
	else if (/*(idLexer::flags & LEXFL_ALLOWPATHNAMES) &&*/ ((c == '/' || c == '\\') || c == '.')) {
		if (!idLexer::ReadName(token)) {
			return 0;
		}
	}
	// check for punctuations
	else if (!idLexer::ReadPunctuation(token)) {
		idLexer::Error("unknown punctuation %c", c);
		return 0;
	}
	// succesfully read a token
	return 1;
}

/*
================
idLexer::ExpectTokenString
================
*/
int idLexer::ExpectTokenString(const std::string& string) {
	idToken token;

	if (!idLexer::ReadToken(&token)) {
		idLexer::Error("couldn't find expected '%s'", string.c_str());
		return 0;
	}
	if (token != string) {
		idLexer::Error("expected '%s' but found '%s'", string.c_str(), token.c_str());
		return 0;
	}
	return 1;
}

/*
================
idLexer::ExpectTokenType
================
*/
int idLexer::ExpectTokenType(int type, int subtype, gsl::not_null<idToken*> token) {
	std::string str;

	if (!idLexer::ReadToken(token)) {
		idLexer::Error("couldn't read expected token");
		return 0;
	}

	if (token->type != type) {
		switch (type) {
		case TT_STRING: str = "string"; break;
		case TT_LITERAL: str = "literal"; break;
		case TT_NUMBER: str = "number"; break;
		case TT_NAME: str = "name"; break;
		case TT_PUNCTUATION: str = "punctuation"; break;
		default: str = "unknown type"; break;
		}
		idLexer::Error("expected a %s but found '%s'", str.c_str(), token->c_str());
		return 0;
	}
	if (token->type == TT_NUMBER) {
		if ((token->subtype & subtype) != subtype) {
			str.clear();
			if (subtype & TT_DECIMAL) str = "decimal ";
			if (subtype & TT_HEX) str = "hex ";
			if (subtype & TT_OCTAL) str = "octal ";
			if (subtype & TT_BINARY) str = "binary ";
			if (subtype & TT_UNSIGNED) str += "unsigned ";
			if (subtype & TT_LONG) str += "long ";
			if (subtype & TT_FLOAT) str += "float ";
			if (subtype & TT_INTEGER) str += "integer ";
			idStr::StripTrailing(str, ' ');
			idLexer::Error("expected %s but found '%s'", str.c_str(), token->c_str());
			return 0;
		}
	}
	/*else if (token->type == TT_PUNCTUATION) {
		if (subtype < 0) {
			idLexer::Error("BUG: wrong punctuation subtype");
			return 0;
		}
		if (token->subtype != subtype) {
			idLexer::Error("expected '%s' but found '%s'", GetPunctuationFromId(subtype), token->c_str());
			return 0;
		}
	}*/
	return 1;
}

/*
================
idLexer::CheckTokenString
================
*/
int idLexer::CheckTokenString(const std::string& string) {
	idToken tok;

	if (!ReadToken(&tok)) {
		return 0;
	}
	// if the given string is available
	if (tok == string) {
		return 1;
	}
	// unread token
	script_p = lastScript_p;
	line = lastline;
	return 0;
}

/*
================
idLexer::CheckTokenType
================
*/
int idLexer::CheckTokenType(int type, int subtype, gsl::not_null<idToken*> token) {
	idToken tok;

	if (!ReadToken(&tok)) {
		return 0;
	}
	// if the type matches
	if (tok.type == type && (tok.subtype & subtype) == subtype) {
		*token = tok;
		return 1;
	}
	// unread token
	script_p = lastScript_p;
	line = lastline;
	return 0;
}

/*
================
idLexer::SkipUntilString
================
*/
int idLexer::SkipUntilString(const std::string& string) {
	idToken token;

	while (idLexer::ReadToken(&token)) {
		if (token == string) {
			return 1;
		}
	}
	return 0;
}

/*
=================
idLexer::SkipBracedSection

Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
int idLexer::SkipBracedSection(bool parseFirstBrace) {
	idToken token;
	int depth;

	depth = parseFirstBrace ? 0 : 1;
	do {
		if (!ReadToken(&token)) {
			return false;
		}
		if (token.type == TT_PUNCTUATION) {
			if (token == "{") {
				depth++;
			}
			else if (token == "}") {
				depth--;
			}
		}
	} while (depth);
	return true;
}

/*
================
idLexer::UnreadToken
================
*/
void idLexer::UnreadToken(gsl::not_null<const idToken*> token) {
	if (idLexer::tokenavailable) {
		common->FatalError("idLexer::unreadToken, unread token twice\n");
	}
	idLexer::token = *token;
	idLexer::tokenavailable = 1;
}

/*
================
idLexer::ReadTokenOnLine
================
*/
int idLexer::ReadTokenOnLine(gsl::not_null<idToken*> token) {
	idToken tok;

	if (!idLexer::ReadToken(&tok)) {
		idLexer::script_p = lastScript_p;
		idLexer::line = lastline;
		return false;
	}
	// if no lines were crossed before this token
	if (!tok.linesCrossed) {
		*token = tok;
		return true;
	}
	// restore our position
	idLexer::script_p = lastScript_p;
	idLexer::line = lastline;
	token->clear();
	return false;
}

/*
================
idLexer::ParseFloat
================
*/
float idLexer::ParseFloat(bool* errorFlag) {
	idToken token;

	if (errorFlag) {
		*errorFlag = false;
	}

	if (!idLexer::ReadToken(&token)) {
		if (errorFlag) {
			idLexer::Warning("couldn't read expected floating point number");
			*errorFlag = true;
		}
		else {
			idLexer::Error("couldn't read expected floating point number");
		}
		return 0;
	}
	if (token.type == TT_PUNCTUATION && token == "-") {
		idLexer::ExpectTokenType(TT_NUMBER, 0, &token);
		return -token.GetFloatValue();
	}
	else if (token.type != TT_NUMBER) {
		if (errorFlag) {
			idLexer::Warning("expected float value, found '%s'", token.c_str());
			*errorFlag = true;
		}
		else {
			idLexer::Error("expected float value, found '%s'", token.c_str());
		}
	}
	return token.GetFloatValue();
}

/*
================
idLexer::Parse1DMatrix
================
*/
int idLexer::Parse1DMatrix(int x, gsl::span<float> m) {
	int i;

	if (!idLexer::ExpectTokenString("(")) {
		return false;
	}

	for (i = 0; i < x; i++) {
		m[i] = idLexer::ParseFloat();
	}

	if (!idLexer::ExpectTokenString(")")) {
		return false;
	}
	return true;
}

/*
================
idLexer::LoadFile
================
*/
int idLexer::LoadFile(const std::string& filename, bool OSPath) {
	std::string pathname;
	int length;
	std::unique_ptr<char[]> buf;

	if (idLexer::loaded) {
		common->Error("idLexer::LoadFile: another script already loaded");
		return false;
	}

	pathname = filename;

	std::shared_ptr<idFile> fp = fileSystem->OpenFileRead(pathname);

	if (!fp) {
		return false;
	}
	length = fp->Length();
	buf = std::make_unique<char[]>(length + 1);
	buf[length] = '\0';
	fp->Read(buf.get(), length);
	idLexer::fileTime = fp->Timestamp();
	idLexer::filename = fp->GetFullPath();
	fileSystem->CloseFile(fp);

	idLexer::bufferUniq = std::move(buf);
	idLexer::buffer = bufferUniq.get();
	idLexer::length = length;
	// pointer in script buffer
	idLexer::script_p = idLexer::buffer;
	// pointer in script buffer before reading token
	idLexer::lastScript_p = idLexer::buffer;
	// pointer to end of script buffer
	idLexer::end_p = &(idLexer::buffer[length]);

	idLexer::tokenavailable = 0;
	idLexer::line = 1;
	idLexer::lastline = 1;
	idLexer::allocated = true;
	idLexer::loaded = true;

	return true;
}

/*
================
idLexer::LoadMemory
================
*/
int idLexer::LoadMemory(const char* ptr, int length, const std::string& name, int startLine) {
	if (idLexer::loaded) {
		common->Error("idLexer::LoadMemory: another script already loaded");
		return false;
	}
	idLexer::filename = name;
	idLexer::buffer = ptr;
	idLexer::fileTime = 0;
	idLexer::length = length;
	// pointer in script buffer
	idLexer::script_p = idLexer::buffer;
	// pointer in script buffer before reading token
	idLexer::lastScript_p = idLexer::buffer;
	// pointer to end of script buffer
	idLexer::end_p = &(idLexer::buffer[length]);

	idLexer::tokenavailable = 0;
	idLexer::line = startLine;
	idLexer::lastline = startLine;
	idLexer::allocated = false;
	idLexer::loaded = true;

	return true;
}

/*
================
idLexer::FreeSource
================
*/
void idLexer::FreeSource() {
	if (idLexer::allocated) {
		idLexer::buffer = nullptr;
		idLexer::allocated = false;
	}
	idLexer::tokenavailable = 0;
	idLexer::token.clear();
	idLexer::loaded = false;
}

/*
================
idLexer::idLexer
================
*/
idLexer::idLexer() {
	idLexer::loaded = false;
	idLexer::filename = "";
	//idLexer::flags = 0;
	idLexer::SetPunctuations(NULL);
	idLexer::allocated = false;
	idLexer::fileTime = 0;
	idLexer::length = 0;
	idLexer::line = 0;
	idLexer::lastline = 0;
	idLexer::tokenavailable = 0;
	idLexer::token.clear();
	//idLexer::next = NULL;
	//idLexer::hadError = false;
}

/*
================
idLexer::idLexer
================
*/
idLexer::idLexer(int flags) {
	idLexer::loaded = false;
	idLexer::filename = "";
	//idLexer::flags = flags;
	idLexer::SetPunctuations(NULL);
	idLexer::allocated = false;
	idLexer::fileTime = 0;
	idLexer::length = 0;
	idLexer::line = 0;
	idLexer::lastline = 0;
	idLexer::tokenavailable = 0;
	idLexer::token.clear();
	//idLexer::next = NULL;
	//idLexer::hadError = false;
}

/*
================
idLexer::~idLexer
================
*/
idLexer::~idLexer() {
	idLexer::FreeSource();
}
