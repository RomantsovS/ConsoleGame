#ifndef FRAMEWORK_CMDSYSTEM_H_
#define FRAMEWORK_CMDSYSTEM_H_

// command flags
enum cmdFlags_t {
  CMD_FL_ALL = -1,
  CMD_FL_CHEAT = BIT(0),     // command is considered a cheat
  CMD_FL_SYSTEM = BIT(1),    // system command
  CMD_FL_RENDERER = BIT(2),  // renderer command
  CMD_FL_SOUND = BIT(3),     // sound command
  CMD_FL_GAME = BIT(4),      // game command
  CMD_FL_TOOL = BIT(5)       // tool command
};

// parameters for command buffer stuffing
enum cmdExecution_t {
  CMD_EXEC_NOW,     // don't return until completed
  CMD_EXEC_INSERT,  // insert at current position, but don't run yet
  CMD_EXEC_APPEND   // add to end of the command buffer (normal case)
};

// command function
using cmdFunction_t = void (*)(const idCmdArgs& args);

class idCmdSystem {
 public:
  idCmdSystem() = default;
  virtual ~idCmdSystem() = default;
  idCmdSystem(const idCmdSystem&) = default;
  idCmdSystem& operator=(const idCmdSystem&) = default;
  idCmdSystem(idCmdSystem&&) = default;
  idCmdSystem& operator=(idCmdSystem&&) = default;

  virtual void Init() = 0;
  virtual void Shutdown() = 0;

  // Registers a command and the function to call for it.
  virtual void AddCommand(
      const std::string& cmdName, cmdFunction_t function, int flags,
      const std::string&
          description /*, argCompletion_t argCompletion = NULL*/) = 0;
  // Remove all commands with one of the flags set.
  virtual void RemoveFlaggedCommands(int flags) = 0;

  virtual void AppendCommandText(const std::string& text) = 0;

  // Adds command text to the command buffer, does not add a final \n
  virtual void BufferCommandText(cmdExecution_t exec,
                                 const std::string& text) = 0;

  // Pulls off \n \r or ; terminated lines of text from the command buffer and
  // executes the commands. Stops when the buffer is empty.
  // Normally called once per frame, but may be explicitly invoked.
  virtual void ExecuteCommandBuffer() = 0;
};

extern idCmdSystem* cmdSystem;

#endif