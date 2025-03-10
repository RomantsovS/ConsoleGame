#ifndef FRAMEWORK_KEYINPUT_H_
#define FRAMEWORK_KEYINPUT_H_

class idKeyInput {
 public:
  static void Init();
  static void Shutdown() noexcept;

  // static void				ArgCompletion_KeyName(const
  // idCmdArgs& args, void(*callback)(const char* s));
  static void PreliminaryKeyEvent(int keyNum, bool down) noexcept;
  static bool IsDown(int keyNum) noexcept;
  static int GetUsercmdAction(int keyNum) noexcept;
  /*static bool				GetOverstrikeMode();
  static void				SetOverstrikeMode(bool state);
  static void				ClearStates();*/

  static keyNum_t StringToKeyNum(
      const std::string& str);  // This is used by the "bind" command
  static const std::string KeyNumToString(
      keyNum_t keyNum);  // This is the inverse of StringToKeyNum, used for
                         // config files
  /*static const char* LocalizedKeyName(keyNum_t keyNum);	// This returns
   * text suitable to print on screen
   */
  static void SetBinding(int keyNum, const std::string& binding);
  /*static const char* GetBinding(int keyNum);
  static bool				UnbindBinding(const char* bind);
  static int				NumBinds(const char* binding);*/
  static bool ExecKeyBinding(int keyNum);
  /*static const char* KeysFromBinding(const char* bind);
  static const char* BindingFromKey(const char* key);
  static bool				KeyIsBoundTo(int keyNum, const char*
  binding); static void				WriteBindings(idFile* f); static
  keyBindings_t	KeyBindingsFromBinding(const char* bind, bool firstOnly = false,
  bool localized = false);*/
};

#endif
