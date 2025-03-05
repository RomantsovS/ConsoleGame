#ifndef GUI_GUI_SCRIPTVAR_H_
#define GUI_GUI_SCRIPTVAR_H_

class idSWFScriptObject;
class idSWFScriptFunction;

/*
========================
A variable in an action script
these can be on the stack, in a script object, passed around as parameters, etc
they can contain raw data (int, float), strings, functions, or objects
========================
*/
class idSWFScriptVar {
 public:
  idSWFScriptVar() : type(swfScriptVarType::SWF_VAR_UNDEF) {}
  idSWFScriptVar(const idSWFScriptVar& other);
  idSWFScriptVar(std::shared_ptr<idSWFScriptObject> o)
      : type(swfScriptVarType::SWF_VAR_UNDEF) {
    SetObject(o);
  }
  idSWFScriptVar(const std::string& s) : type(swfScriptVarType::SWF_VAR_UNDEF) {
    SetString(s);
  }
  idSWFScriptVar(const char* s) : type(swfScriptVarType::SWF_VAR_UNDEF) {
    SetString(std::string(s));
  }
  idSWFScriptVar(int i) : type(swfScriptVarType::SWF_VAR_UNDEF) {
    SetInteger(i);
  }
  idSWFScriptVar(std::shared_ptr<idSWFScriptFunction> nf)
      : type(swfScriptVarType::SWF_VAR_UNDEF) {
    SetFunction(nf);
  }
  ~idSWFScriptVar();

  idSWFScriptVar& operator=(const idSWFScriptVar& other) noexcept;
  idSWFScriptVar(idSWFScriptVar&&) = default;
  idSWFScriptVar& operator=(idSWFScriptVar&&) = default;

  void SetString(const std::string& s) {
    Free();
    type = swfScriptVarType::SWF_VAR_STRING;
    value.string = std::make_shared<std::string>(s);
  }
  void SetFloat(float f) noexcept {
    Free();
    type = swfScriptVarType::SWF_VAR_FLOAT;
    value.f = f;
  }
  void SetBool(bool b) noexcept {
    Free();
    type = swfScriptVarType::SWF_VAR_BOOL;
    value.b = b;
  }
  void SetInteger(int i) noexcept {
    Free();
    type = swfScriptVarType::SWF_VAR_INTEGER;
    value.i = i;
  }

  void SetObject(std::shared_ptr<idSWFScriptObject> o) noexcept;
  void SetFunction(std::shared_ptr<idSWFScriptFunction> f) noexcept;

  std::string ToString() const;
  bool ToBool() const;
  int ToInteger() const;

  std::shared_ptr<idSWFScriptObject> GetObjectScript() noexcept {
    return value.object.lock();
  }
  std::shared_ptr<idSWFScriptObject> GetObjectScript() const noexcept {
    return value.object.lock();
  }
  std::shared_ptr<idSWFScriptFunction> GetFunction() noexcept {
    return value.function;
  }
  std::shared_ptr<idSWFSpriteInstance> ToSprite() noexcept;
  std::shared_ptr<idSWFTextInstance> ToText() noexcept;

  bool IsString() const noexcept {
    return (type == swfScriptVarType::SWF_VAR_STRING) ||
           (type == swfScriptVarType::SWF_VAR_STRINGID);
  }
  bool IsFunction() const noexcept {
    return (type == swfScriptVarType::SWF_VAR_FUNCTION);
  }
  bool IsObject() const noexcept {
    return (type == swfScriptVarType::SWF_VAR_OBJECT);
  }

  enum class swfScriptVarType {
    SWF_VAR_STRINGID,
    SWF_VAR_STRING,
    SWF_VAR_FLOAT,
    SWF_VAR_NULL,
    SWF_VAR_UNDEF,
    SWF_VAR_BOOL,
    SWF_VAR_INTEGER,
    SWF_VAR_FUNCTION,
    SWF_VAR_OBJECT
  };

  swfScriptVarType GetType() const noexcept { return type; }

 private:
  void Free() noexcept;
  swfScriptVarType type;

  struct idSWFScriptVarValue_t {
    idSWFScriptVarValue_t() {}
    ~idSWFScriptVarValue_t() {}

    idSWFScriptVarValue_t& operator=(
        const idSWFScriptVarValue_t& other) noexcept {
      if (this != &other) {
        f = other.f;
        i = other.i;
        b = other.b;
        object = other.object;
        string = other.string;
        function = other.function;
      }
      return *this;
    }

    idSWFScriptVarValue_t(const idSWFScriptVarValue_t&) = default;
    idSWFScriptVarValue_t(idSWFScriptVarValue_t&&) = default;
    idSWFScriptVarValue_t& operator=(idSWFScriptVarValue_t&&) = default;

    float f;
    int i;
    bool b;
    std::weak_ptr<idSWFScriptObject> object;
    std::shared_ptr<std::string> string;
    std::shared_ptr<idSWFScriptFunction> function;
  } value;
};
#endif
