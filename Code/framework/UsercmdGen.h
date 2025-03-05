#ifndef FRAMEWORK_USERCMDGEN_H_
#define FRAMEWORK_USERCMDGEN_H_

#include "../sys/sys_session.h"

/*
===============================================================================

        Samples a set of user commands from player input.

===============================================================================
*/

// usercmd_t->button bits
const int BUTTON_ATTACK = BIT(0);
const int BUTTON_RUN = BIT(1);
const int BUTTON_ZOOM = BIT(2);
const int BUTTON_SCORES = BIT(3);
const int BUTTON_USE = BIT(4);
const int BUTTON_JUMP = BIT(5);
const int BUTTON_CROUCH = BIT(6);
const int BUTTON_CHATTING = BIT(7);

// usercmd_t->impulse commands
const int IMPULSE_0 = 0;    // weap 0
const int IMPULSE_1 = 1;    // weap 1
const int IMPULSE_2 = 2;    // weap 2
const int IMPULSE_3 = 3;    // weap 3
const int IMPULSE_4 = 4;    // weap 4
const int IMPULSE_5 = 5;    // weap 5
const int IMPULSE_6 = 6;    // weap 6
const int IMPULSE_7 = 7;    // weap 7
const int IMPULSE_8 = 8;    // weap 8
const int IMPULSE_9 = 9;    // weap 9
const int IMPULSE_10 = 10;  // weap 10
const int IMPULSE_11 = 11;  // weap 11
const int IMPULSE_12 = 12;  // weap 12
const int IMPULSE_13 = 13;  // weap reload
const int IMPULSE_14 = 14;  // weap next
const int IMPULSE_15 = 15;  // weap prev
const int IMPULSE_16 = 16;  // toggle flashlight on/off
const int IMPULSE_18 = 18;  // center view
const int IMPULSE_19 = 19;  // show PDA/SCORES
const int IMPULSE_22 = 22;  // spectate
const int IMPULSE_25 = 25;  // Envirosuit light
const int IMPULSE_27 = 27;  // Chainsaw
const int IMPULSE_28 = 28;  // quick 0
const int IMPULSE_29 = 29;  // quick 1
const int IMPULSE_30 = 30;  // quick 2
const int IMPULSE_31 = 31;  // quick 3

class usercmd_t {
 public:
  usercmd_t()
      : forwardmove(),
        rightmove(),
        buttons(),
        /*clientGameMilliseconds(0),
        serverGameMilliseconds(0),
        fireCount(0),
        mx(),
        my(),*/
        impulse()
  /*impulseSequence(),
  pos(0.0f, 0.0f, 0.0f),
  speedSquared(0.0f)*/
  {
    /*angles[0] = 0;
    angles[1] = 0;
    angles[2] = 0;*/
  }

  // Syncronized
  // short		angles[3];
  // // view angles
  signed char forwardmove;  // forward/backward movement
  signed char rightmove;    // left/right movement

  unsigned char buttons;  // buttons
  /*int			clientGameMilliseconds;			// time this
  usercmd was sent from the client int			serverGameMilliseconds;
  // interpolated server time this was applied on uint16
  fireCount;						// number of times we've
  fired
*/
  // Not syncronized
  unsigned char impulse;  // impulse command
                          /*byte		impulseSequence;				//
                          incremented every time there's a new impulse
                        
                          short		mx;
                          // mouse delta x                         short		my;
                          // mouse delta y
                        
                          // Clients are authoritative on their positions
                          idVec3		pos;
                          float		speedSquared;*/

 public:
  /*void		Serialize(class idSerializer& s, const usercmd_t& base);
  void		ByteSwap();						// on
  big endian systems, byte swap the shorts and ints bool
  operator==(const usercmd_t& rhs) const;*/
};

enum class inhibit_t { INHIBIT_SESSION = 0, INHIBIT_ASYNC };

enum usercmdButton_t {
  UB_NONE,

  UB_MOVEUP,
  UB_MOVEDOWN,
  UB_LOOKLEFT,
  UB_LOOKRIGHT,
  UB_MOVEFORWARD,
  UB_MOVEBACK,
  UB_LOOKUP,
  UB_LOOKDOWN,
  UB_MOVELEFT,
  UB_MOVERIGHT,

  UB_ATTACK,
  UB_SPEED,
  UB_ZOOM,
  UB_SHOWSCORES,
  UB_USE,

  UB_IMPULSE0,
  UB_IMPULSE1,
  UB_IMPULSE2,
  UB_IMPULSE3,
  UB_IMPULSE4,
  UB_IMPULSE5,
  UB_IMPULSE6,
  UB_IMPULSE7,
  UB_IMPULSE8,
  UB_IMPULSE9,
  UB_IMPULSE10,
  UB_IMPULSE11,
  UB_IMPULSE12,
  UB_IMPULSE13,
  UB_IMPULSE14,
  UB_IMPULSE15,
  UB_IMPULSE16,
  UB_IMPULSE17,
  UB_IMPULSE18,
  UB_IMPULSE19,
  UB_IMPULSE20,
  UB_IMPULSE21,
  UB_IMPULSE22,
  UB_IMPULSE23,
  UB_IMPULSE24,
  UB_IMPULSE25,
  UB_IMPULSE26,
  UB_IMPULSE27,
  UB_IMPULSE28,
  UB_IMPULSE29,
  UB_IMPULSE30,
  UB_IMPULSE31,

  UB_MAX_BUTTONS
};

struct userCmdString_t {
  const std::string string;
  usercmdButton_t button;
};

class idUsercmdGen {
 public:
  idUsercmdGen() = default;
  virtual ~idUsercmdGen() = default;
  idUsercmdGen(const idUsercmdGen&) = default;
  idUsercmdGen& operator=(const idUsercmdGen&) = default;
  idUsercmdGen(idUsercmdGen&&) = default;
  idUsercmdGen& operator=(idUsercmdGen&&) = default;

  // Sets up all the cvars and console commands.
  virtual void Init() = 0;

  // Prepares for a new map.
  virtual void InitForNewMap() = 0;

  // Shut down.
  virtual void Shutdown() = 0;

  // Clears all key states and face straight.
  virtual void Clear() = 0;

  // Clears view angles.
  // virtual void		ClearAngles() = 0;

  // When the console is down or the menu is up, only emit default usercmd, so
  // the player isn't moving around. Each subsystem (session and game) may want
  // an inhibit will OR the requests.
  virtual void InhibitUsercmd(inhibit_t subsystem, bool inhibit) = 0;

  // Set a value that can safely be referenced by UsercmdInterrupt() for each
  // key binding.
  virtual int CommandStringUsercmdData(const std::string& cmdString) = 0;

  // Continuously modified, never reset. For full screen guis.
  // virtual void		MouseState(int* x, int* y, int* button, bool*
  // down) = 0;

  // Directly sample a button.
  virtual int ButtonState(int key) = 0;

  // Directly sample a keystate.
  virtual int KeyState(int key) = 0;

  // called at vsync time
  virtual void BuildCurrentUsercmd(int deviceNum) = 0;

  // return the current usercmd
  virtual usercmd_t GetCurrentUsercmd() = 0;
};

extern idUsercmdGen* usercmdGen;
// extern userCmdString_t	userCmdStrings[];

/*
================================================
idUserCmdMgr
================================================
*/
class idUserCmdMgr {
 public:
  idUserCmdMgr() { SetDefaults(); }

  void SetDefaults() {
    for (int i = 0; i < cmdBuffer.size(); ++i) {
      cmdBuffer[i].fill({});
    }
    writeFrame.fill(0);
    readFrame.fill(-1);
  }

  // Set to 128 for now
  // Temp fix for usercmds overflowing  Correct fix is to process usercmds as
  // they come in (like q3), rather then buffer them up.
  static const int USERCMD_BUFFER_SIZE = 128;

  std::array<std::array<usercmd_t, MAX_PLAYERS>, USERCMD_BUFFER_SIZE> cmdBuffer;
  std::array<int, MAX_PLAYERS> writeFrame;  //"where we write to next"
  std::array<int, MAX_PLAYERS> readFrame;   //"the last frame we read"

  void PutUserCmdForPlayer(int playerIndex, const usercmd_t& cmd) {
    cmdBuffer[writeFrame[playerIndex] % USERCMD_BUFFER_SIZE][playerIndex] = cmd;
    if (writeFrame[playerIndex] - readFrame[playerIndex] + 1 >
        USERCMD_BUFFER_SIZE) {
      readFrame[playerIndex] =
          writeFrame[playerIndex] -
          USERCMD_BUFFER_SIZE /
              2;  // Set to middle of buffer as a temp fix until we can catch
                  // the client up correctly
      idLib::Printf("PutUserCmdForPlayer: buffer overflow.\n");
    }
    writeFrame[playerIndex]++;
  }

  void ResetPlayer(int playerIndex) {
    for (int i = 0; i < USERCMD_BUFFER_SIZE; i++) {
      memset(&cmdBuffer[i][playerIndex], 0, sizeof(usercmd_t));
    }
    writeFrame[playerIndex] = 0;
    readFrame[playerIndex] = -1;
  }

  bool HasUserCmdForPlayer(int playerIndex, int buffer = 0) const {
    // return true if the last frame we read from (+ buffer) is < the last frame
    // we wrote to (remember writeFrame is where we write to *next*. readFrame
    // is where we last read from last)
    bool hasCmd =
        (readFrame[playerIndex] + buffer < writeFrame[playerIndex] - 1);
    return hasCmd;
  }

  const usercmd_t& GetUserCmdForPlayer(int playerIndex) {
    // Get the next cmd we should process (not necessarily the newest)
    // Note we may have multiple reads for every write .
    // We want to:
    //  A) never skip over a cmd (unless we call MakeReadPtrCurrentForPlayer()
    //  ). B) never get ahead of the writeFrame

    // try to increment before reading (without this we may read the same input
    // twice and be a frame behind our writes in the case of)
    if (readFrame[playerIndex] < writeFrame[playerIndex] - 1) {
      readFrame[playerIndex]++;
    }

    // grab the next command in the readFrame buffer
    int index = readFrame[playerIndex];
    usercmd_t& result = cmdBuffer[index % USERCMD_BUFFER_SIZE][playerIndex];
    return result;
  }

  // Hack to let the player inject his position into the correct usercmd.
  usercmd_t& GetWritableUserCmdForPlayer(int playerIndex) {
    // Get the next cmd we should process (not necessarily the newest)
    // Note we may have multiple reads for every write .
    // We want to:
    //  A) never skip over a cmd (unless we call MakeReadPtrCurrentForPlayer()
    //  ). B) never get ahead of the writeFrame

    // try to increment before reading (without this we may read the same input
    // twice and be a frame behind our writes in the case of)
    if (readFrame[playerIndex] < writeFrame[playerIndex] - 1) {
      readFrame[playerIndex]++;
    }

    // grab the next command in the readFrame buffer
    int index = readFrame[playerIndex];
    usercmd_t& result = cmdBuffer[index % USERCMD_BUFFER_SIZE][playerIndex];
    return result;
  }

  void MakeReadPtrCurrentForPlayer(int playerIndex) {
    // forces us to the head of our read buffer. As if we have processed every
    // cmd available to us and now HasUserCmdForPlayer() returns FALSE Note we
    // do -1 to point us to the last written cmd. If a read before the next
    // write, you will get the last write. (not garbage) If a write is made
    // before the next read, you *will* get the new write ( b/c
    // GetUserCmdForPlayer pre increments)

    // After calling this, HasUserCmdForPlayer() will return FALSE;
    readFrame[playerIndex] = writeFrame[playerIndex] - 1;
  }

  int GetPlayerCmds(int user, usercmd_t** buffer, const int bufferSize) {
    // Fallback to getting cmds from the userCmdMgr
    int start = std::max(
        writeFrame[user] - std::min(bufferSize, USERCMD_BUFFER_SIZE), 0);
    int numCmds = writeFrame[user] - start;

    for (int i = 0; i < numCmds; i++) {
      int index = (start + i) % USERCMD_BUFFER_SIZE;
      buffer[i] = &cmdBuffer[index][user];
    }
    return numCmds;
  }
};

#endif
