#ifndef IDLIB_TEXT_CMDARGS_H_
#define IDLIB_TEXT_CMDARGS_H_

#include "Token.h"

class idCmdArgs {
 public:
  idCmdArgs() = default;
  idCmdArgs(const std::string& text, bool keepAsStrings) {
    TokenizeString(text, keepAsStrings);
  }
  ~idCmdArgs() = default;
  void operator=(const idCmdArgs& args);
  idCmdArgs(const idCmdArgs&) = default;
  idCmdArgs(idCmdArgs&&) = default;
  idCmdArgs& operator=(idCmdArgs&&) = default;

  // The functions that execute commands get their parameters with these
  // functions.
  int Argc() const noexcept { return argv.size(); }
  // Argv() will return an empty string, not NULL if arg >= argc.
  const idToken Argv(size_t arg) const {
    return (arg >= 0 && arg < argv.size()) ? argv[arg] : idToken();
  }
  // Returns a single string containing argv(start) to argv(end)
  // escapeArgs is a fugly way to put the string back into a state ready to
  // tokenize again
  const std::string Args(size_t start = 1, size_t end = -1,
                         bool escapeArgs = false) const;

  // Takes a null terminated string and breaks the string up into arg tokens.
  // Does not need to be /n terminated.
  // Set keepAsStrings to true to only seperate tokens from whitespace and
  // comments, ignoring punctuation
  void TokenizeString(const std::string& text, bool keepAsStrings);

  // void AppendArg(std::string &text);
  void Clear() noexcept { argv.clear(); }
  // const std::vector<std::string>& GetArgs();

 private:
  std::string CutQuotes(const std::string& str);
  static const int MAX_COMMAND_ARGS = 64;
  static const int MAX_COMMAND_STRING = 2 * max_string_chars;

  // int argc; // number of arguments
  std::vector<idToken> argv;  // points into tokenized
  // std::string tokenized;		// will have 0 bytes inserted
};

#endif
