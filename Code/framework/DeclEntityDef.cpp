#include "idlib/precompiled.h"

/*
================
idDeclEntityDef::FreeData
================
*/
void idDeclEntityDef::FreeData() noexcept { dict.Clear(); }

/*
================
idDeclEntityDef::Parse
================
*/
bool idDeclEntityDef::Parse(const char* text, const int textLength,
                            bool allowBinaryVersion) {
  idLexer src;
  idToken token, token2;

  src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
  // src.SetFlags(DECL_LEXER_FLAGS);
  src.SkipUntilString("{");

  while (1) {
    if (!src.ReadToken(&token)) {
      break;
    }

    if (token == "}") {
      break;
    }
    if (token.type != TT_STRING) {
      src.Warning("Expected quoted string, but found '%s'", token.c_str());
      MakeDefault();
      return false;
    }

    if (!src.ReadToken(&token2)) {
      src.Warning("Unexpected end of file");
      MakeDefault();
      return false;
    }

    if (dict.FindKey(token) != "") {
      src.Warning("'%s' already defined", token.c_str());
    }
    dict.Set(token, token2);
  }

  // we always automatically set a "classname" key to our name
  dict.Set("classname", GetName());

  game->CacheDictionaryMedia(&dict);

  return true;
}

/*
================
idDeclEntityDef::DefaultDefinition
================
*/
std::string idDeclEntityDef::DefaultDefinition() const {
  return "{\n"
         "\t"
         "\"DEFAULTED\"\t\"1\"\n"
         "}";
}
