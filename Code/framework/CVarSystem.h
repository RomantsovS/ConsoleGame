#ifndef FRAMEWORK_CVAR_SYSTEM_H_
#define FRAMEWORK_CVAR_SYSTEM_H_

enum cvarFlags_t {
  CVAR_ALL = -1,               // all flags
  CVAR_BOOL = BIT(0),          // variable is a boolean
  CVAR_INTEGER = BIT(1),       // variable is an integer
  CVAR_FLOAT = BIT(2),         // variable is a float
  CVAR_SYSTEM = BIT(3),        // system variable
  CVAR_RENDERER = BIT(4),      // renderer variable
  CVAR_SOUND = BIT(5),         // sound variable
  CVAR_GUI = BIT(6),           // gui variable
  CVAR_GAME = BIT(7),          // game variable
  CVAR_TOOL = BIT(8),          // tool variable
  CVAR_SERVERINFO = BIT(10),   // sent from servers, available to menu
  CVAR_NETWORKSYNC = BIT(11),  // cvar is synced from the server to clients
  CVAR_STATIC = BIT(12),       // statically declared, not user created
  CVAR_CHEAT = BIT(13),        // variable is considered a cheat
  CVAR_NOCHEAT = BIT(14),      // variable is not considered a cheat
  CVAR_INIT = BIT(15),         // can only be set from the command-line
  CVAR_ROM = BIT(16),          // display only, cannot be set by user at all
  CVAR_ARCHIVE = BIT(17),      // set to cause it to be saved to a config file
  CVAR_MODIFIED = BIT(18)      // set when the variable is modified
};

/*
===============================================================================

        idCVar

===============================================================================
*/

class idCVar {
 public:
  // Never use the default constructor.
  idCVar() { idassert(typeid(this) != typeid(idCVar)); }

  // Always use one of the following constructors.
  idCVar(const std::string& name, const std::string& value, int flags,
         const std::string& description);
  idCVar(const std::string& name, const std::string& value, int flags,
         const std::string& description, float valueMin, float valueMax);

  virtual ~idCVar() = default;
  idCVar(const idCVar&) = default;
  idCVar& operator=(const idCVar&) = default;
  idCVar(idCVar&&) = default;
  idCVar& operator=(idCVar&&) = default;

  const std::string& GetName() const noexcept { return internalVar->name; }
  std::string GetName() { return internalVar->name; }
  int GetFlags() const noexcept { return internalVar->flags; }
  const std::string& GetDescription() const noexcept {
    return internalVar->description;
  }
  float GetMinValue() const noexcept { return internalVar->valueMin; }
  float GetMaxValue() const noexcept { return internalVar->valueMax; }

  bool IsModified() const noexcept {
    return (internalVar->flags & CVAR_MODIFIED) != 0;
  }
  void SetModified() noexcept { internalVar->flags |= CVAR_MODIFIED; }
  void ClearModified() noexcept { internalVar->flags &= ~CVAR_MODIFIED; }

  const std::string& GetDefaultString() const noexcept {
    return internalVar->InternalGetResetString();
  }
  const std::string& GetString() const noexcept { return internalVar->value; }
  bool GetBool() const noexcept { return (internalVar->integerValue != 0); }
  int GetInteger() const noexcept { return internalVar->integerValue; }
  float GetFloat() const noexcept { return internalVar->floatValue; }

  void SetString(const char* value) { internalVar->InternalSetString(value); }
  void SetString(const std::string& value) {
    internalVar->InternalSetString(value);
  }
  void SetBool(const bool value) noexcept {
    internalVar->InternalSetBool(value);
  }
  void SetInteger(const int value) noexcept {
    internalVar->InternalSetInteger(value);
  }
  void SetFloat(const float value) noexcept {
    internalVar->InternalSetFloat(value);
  }

  void SetInternalVar(idCVar* cvar) noexcept { internalVar = cvar; }

  static void RegisterStaticVars();

 protected:
  std::string name;         // name
  std::string value;        // value
  std::string description;  // description
  int flags;                // CVAR_? flags
  float valueMin;           // minimum value
  float valueMax;           // maximum value
  int integerValue;         // atoi( string )
  float floatValue;         // atof( value )
  idCVar* internalVar;      // internal cvar
  idCVar* next;             // next statically declared cvar

 private:
  void Init(const std::string& name, const std::string& value, int flags,
            const std::string& description, float valueMin, float valueMax,
            const char** valueStrings);

  virtual void InternalSetString(const std::string& newValue) noexcept {}
  virtual void InternalSetBool(const bool newValue) noexcept {}
  virtual void InternalSetInteger(const int newValue) noexcept {}
  virtual void InternalSetFloat(const float newValue) noexcept {}

  virtual const std::string& InternalGetResetString() const noexcept {
    return value;
  }

  static idCVar* staticVars;
};

inline idCVar::idCVar(const std::string& name, const std::string& value,
                      int flags, const std::string& description) {
  Init(name, value, flags, description, 1, -1, NULL);
}

inline idCVar::idCVar(const std::string& name, const std::string& value,
                      int flags, const std::string& description, float valueMin,
                      float valueMax) {
  Init(name, value, flags, description, valueMin, valueMax, NULL);
}

/*
===============================================================================

        idCVarSystem

===============================================================================
*/

class idCVarSystem {
 public:
  idCVarSystem() = default;
  virtual ~idCVarSystem() = default;
  idCVarSystem(const idCVarSystem&) = default;
  idCVarSystem& operator=(const idCVarSystem&) = default;
  idCVarSystem(idCVarSystem&&) = default;
  idCVarSystem& operator=(idCVarSystem&&) = default;

  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual bool IsInitialized() const = 0;

  // Registers a CVar.
  virtual void Register(gsl::not_null<idCVar*> cvar) = 0;

  // Called by the command system when argv(0) doesn't match a known command.
  // Returns true if argv(0) is a variable reference and prints or changes the
  // CVar.
  virtual bool Command(const idCmdArgs& args) = 0;

  // Sets the value of a CVar by name.
  virtual void SetCVarString(const std::string& name, const std::string& value,
                             int flags = 0) = 0;

  virtual std::string GetCVarString(const std::string& name) const = 0;

  // Sets/gets/clears modified flags that tell what kind of CVars have changed.
  virtual void SetModifiedFlags(int flags) = 0;
};

extern idCVarSystem* cvarSystem;

/*
===============================================================================

        CVar Registration

        Each DLL using CVars has to declare a private copy of the static
variable idCVar::staticVars like this: idCVar * idCVar::staticVars = NULL;
        Furthermore idCVar::RegisterStaticVars() has to be called after the
        cvarSystem pointer is set when the DLL is first initialized.

===============================================================================
*/

inline void idCVar::Init(const std::string& name, const std::string& value,
                         int flags, const std::string& description,
                         float valueMin, float valueMax,
                         const char** valueStrings) {
  this->name = name;
  this->value = value;
  this->flags = flags;
  this->description = description;
  this->flags = flags | CVAR_STATIC;
  this->valueMin = valueMin;
  this->valueMax = valueMax;
  // this->valueStrings = valueStrings;
  // this->valueCompletion = valueCompletion;
  this->integerValue = 0;
  this->floatValue = 0.0f;
  this->internalVar = this;
  if (staticVars != (idCVar*)0xFFFFFFFF) {
    this->next = staticVars;
    staticVars = this;
  } else {
    cvarSystem->Register(this);
  }
}

inline void idCVar::RegisterStaticVars() {
  if (staticVars != (idCVar*)0xFFFFFFFF) {
    for (auto cvar = staticVars; cvar; cvar = cvar->next) {
      cvarSystem->Register(cvar);
    }
    staticVars = (idCVar*)0xFFFFFFFF;
  }
}

#endif