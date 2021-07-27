#ifndef IDLIB_TEXT_LEXER_H_
#define IDLIB_TEXT_LEXER_H_

// punctuation ids
#define P_RSHIFT_ASSIGN				1
#define P_LSHIFT_ASSIGN				2
#define P_PARMS						3
#define P_PRECOMPMERGE				4

#define P_LOGIC_AND					5
#define P_LOGIC_OR					6
#define P_LOGIC_GEQ					7
#define P_LOGIC_LEQ					8
#define P_LOGIC_EQ					9
#define P_LOGIC_UNEQ				10

#define P_MUL_ASSIGN				11
#define P_DIV_ASSIGN				12
#define P_MOD_ASSIGN				13
#define P_ADD_ASSIGN				14
#define P_SUB_ASSIGN				15
#define P_INC						16
#define P_DEC						17

#define P_BIN_AND_ASSIGN			18
#define P_BIN_OR_ASSIGN				19
#define P_BIN_XOR_ASSIGN			20
#define P_RSHIFT					21
#define P_LSHIFT					22

#define P_POINTERREF				23
#define P_CPP1						24
#define P_CPP2						25
#define P_MUL						26
#define P_DIV						27
#define P_MOD						28
#define P_ADD						29
#define P_SUB						30
#define P_ASSIGN					31

#define P_BIN_AND					32
#define P_BIN_OR					33
#define P_BIN_XOR					34
#define P_BIN_NOT					35

#define P_LOGIC_NOT					36
#define P_LOGIC_GREATER				37
#define P_LOGIC_LESS				38

#define P_REF						39
#define P_COMMA						40
#define P_SEMICOLON					41
#define P_COLON						42
#define P_QUESTIONMARK				43

#define P_PARENTHESESOPEN			44
#define P_PARENTHESESCLOSE			45
#define P_BRACEOPEN					46
#define P_BRACECLOSE				47
#define P_SQBRACKETOPEN				48
#define P_SQBRACKETCLOSE			49
#define P_BACKSLASH					50

#define P_PRECOMP					51
#define P_DOLLAR					52

// punctuation
struct punctuation_t
{
	const char* p;						// punctuation character(s)
	int n;							// punctuation id
};

class idLexer {

	//friend class idParser;

public:
	// constructor
	idLexer();
	idLexer(int flags);
	// destructor
	~idLexer();
	idLexer(const idLexer&) = default;
	idLexer& operator=(const idLexer&) = default;
	idLexer(idLexer&&) = default;
	idLexer& operator=(idLexer&&) = default;

	// load a script from the given file at the given offset with the given length
	int LoadFile(const std::string& filename, bool OSPath = false);
	// load a script from the given memory with the given length and a specified line offset,
	// so source strings extracted from a file can still refer to proper line numbers in the file
	// NOTE: the ptr is expected to point at a valid C string: ptr[length] == '\0'
	int				LoadMemory(const char* ptr, int length, const std::string& name, int startLine = 1);
	// free the script
	void			FreeSource() noexcept;
	// returns true if a script is loaded
	int				IsLoaded() noexcept { return idLexer::loaded; };
	// read a token
	int				ReadToken(gsl::not_null<idToken*> token);
	// expect a certain token, reads the token when available
	int				ExpectTokenString(const std::string& string);
	// expect a certain token type
	int				ExpectTokenType(int type, int subtype, gsl::not_null<idToken*> token);
	// returns true when the token is available
	int				CheckTokenString(const std::string& string);
	// returns true an reads the token when a token with the given type is available
	int				CheckTokenType(int type, int subtype, gsl::not_null<idToken*> token);
	// skip tokens until the given token string is read
	int				SkipUntilString(const std::string& string);
	// skip the braced section
	int				SkipBracedSection(bool parseFirstBrace = true);
	// unread the given token
	void			UnreadToken(gsl::not_null<const idToken*> token);
	// read a token only if on the same line
	int				ReadTokenOnLine(gsl::not_null<idToken*> token);

	// read a floating point number.  If errorFlag is NULL, a non-numeric token will
	// issue an Error().  If it isn't NULL, it will issue a Warning() and set *errorFlag = true
	float			ParseFloat(bool* errorFlag = nullptr);
	// parse matrices with floats
	int				Parse1DMatrix(int x, gsl::span<float> m);
	// set an array with punctuations, NULL restores default C/C++ set, see default_punctuations for an example
	void			SetPunctuations(const std::vector<punctuation_t>* p);
	// get offset in script
	const int		GetFileOffset() noexcept;
	// get file time
	const ID_TIME_T	GetFileTime() noexcept;
	// returns the current line number
	const int		GetLineNum() noexcept;
	// print an error message
	void			Error(const char* str, ...);
	// print a warning message
	void			Warning(const char* str, ...);
private:
	int				loaded;					// set when a script file is loaded from file or memory
	std::string			filename;				// file name of the script
	int				allocated;				// true if buffer memory was allocated
	const char* buffer;					// buffer containing the script
	std::unique_ptr<char[]> bufferUniq;
	const char* script_p;				// current pointer in the script
	const char* end_p;					// pointer to the end of the script
	const char* lastScript_p;			// script pointer before reading token
	ID_TIME_T			fileTime;				// file time
	int				length;					// length of the script in bytes
	int				line;					// current line in script
	int				lastline;				// line before reading token
	int				tokenavailable;			// set by unreadToken
	const std::vector<punctuation_t>* punctuations;		// the punctuations used in the script
	std::vector<int>* punctuationtable;		// ASCII table with punctuations
	std::vector<int>* nextpunctuation;		// next punctuation in chain
	idToken			token;					// available token
private:
	void			CreatePunctuationTable(const std::vector<punctuation_t>& punctuations);
	int				ReadWhiteSpace();
	int				ReadString(gsl::not_null<idToken*> token, int quote);
	int				ReadName(gsl::not_null<idToken*> token);
	int				ReadNumber(gsl::not_null<idToken*> token);
	int				ReadPunctuation(gsl::not_null<idToken*> token);
};

inline const int idLexer::GetFileOffset() noexcept {
	return idLexer::script_p - idLexer::buffer;
}

inline const ID_TIME_T idLexer::GetFileTime() noexcept {
	return idLexer::fileTime;
}

inline const int idLexer::GetLineNum() noexcept {
	return idLexer::line;
}

#endif // !IDLIB_TEXT_LEXER_H_
