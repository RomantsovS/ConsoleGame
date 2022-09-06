#ifndef SYS_PUBLIC_H_
#define SYS_PUBLIC_H_

#include "../idlib/CmdArgs.h"

enum sysEventType_t {
	SE_NONE,				// evTime is still valid
	SE_KEY,					// evValue is a key code, evValue2 is the down flag
	SE_CHAR,				// evValue is an ascii char
	SE_MOUSE,				// evValue and evValue2 are reletive signed x / y moves
	SE_MOUSE_ABSOLUTE,		// evValue and evValue2 are absolute coordinates in the window's client area.
	SE_MOUSE_LEAVE,			// evValue and evValue2 are meaninless, this indicates the mouse has left the client area.
	SE_JOYSTICK,		// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE				// evPtr is a char*, from typing something at a non-game console
};

/*
================================================
The first part of this table maps directly to Direct Input scan codes (DIK_* from dinput.h)
But they are duplicated here for console portability
================================================
*/
enum class keyNum_t {
	K_NONE,

	K_BACKSPACE = 8,
	K_TAB = 9,
	K_ENTER = 13,
	K_LSHIFT = 16,
	K_ESCAPE = 27,
	K_SPACE = 32,
	K_UPARROW = 38,
	K_LEFTARROW = 37,
	K_RIGHTARROW = 39,
	K_DOWNARROW = 40,
	K_1,
	K_2,
	K_3,
	K_4,
	K_5,
	K_6,
	K_7,
	K_8,
	K_9,
	K_0,
	K_MINUS,
	K_EQUALS,
	K_Q,
	K_W,
	K_E,
	K_R,
	K_T,
	K_Y,
	K_U,
	K_I,
	K_O,
	K_P,
	K_LBRACKET,
	K_RBRACKET,
	K_LCTRL,
	K_A,
	K_S,
	K_D,
	K_F,
	K_G,
	K_H,
	K_J,
	K_K,
	K_L,
	K_SEMICOLON,
	K_APOSTROPHE,
	K_F1 = 112,
	K_F2,
	K_F3,
	K_F4,
	K_F5,
	K_F6,
	K_F7,
	K_F8,
	K_F9,
	K_F10,
	K_GRAVE = 96,
	K_BACKSLASH,
	K_Z,
	K_X,
	K_C,
	K_V,
	K_B,
	K_N,
	K_M,
	K_COMMA,
	K_PERIOD,
	K_SLASH,
	K_RSHIFT,
	K_KP_STAR,
	K_LALT,
	K_CAPSLOCK,
	K_NUMLOCK,
	K_SCROLL,
	K_KP_7,
	K_KP_8,
	K_KP_9,
	K_KP_MINUS,
	K_KP_4,
	K_KP_5,
	K_KP_6,
	K_KP_PLUS,
	K_KP_1,
	K_KP_2,
	K_KP_3,
	K_KP_0,
	K_KP_DOT,
	K_F11 = 0x57,
	K_F12 = 0x58,
	K_F13 = 0x64,
	K_F14 = 0x65,
	K_F15 = 0x66,
	K_KANA = 0x70,
	K_CONVERT = 0x79,
	K_NOCONVERT = 0x7B,
	K_YEN = 0x7D,
	K_KP_EQUALS = 0x8D,
	K_CIRCUMFLEX = 0x90,
	K_AT = 0x91,
	K_COLON = 0x92,
	K_UNDERLINE = 0x93,
	K_KANJI = 0x94,
	K_STOP = 0x95,
	K_AX = 0x96,
	K_UNLABELED = 0x97,
	K_KP_ENTER = 0x9C,
	K_RCTRL = 0x9D,
	K_KP_COMMA = 0xB3,
	K_KP_SLASH = 0xB5,
	K_PRINTSCREEN = 0xB7,
	K_RALT = 0xB8,
	K_PAUSE = 0xC5,
	K_HOME = 0xC7,
	K_PGUP = 0xC9,
	K_END = 0xCF,
	K_PGDN = 0xD1,
	K_INS = 0xD2,
	K_DEL = 0xD3,
	K_LWIN = 0xDB,
	K_RWIN = 0xDC,
	K_APPS = 0xDD,
	K_POWER = 0xDE,
	K_SLEEP = 0xDF,

	//------------------------
	// K_MOUSE enums must be contiguous (no char codes in the middle)
	//------------------------

	K_MOUSE1,
	K_MOUSE2,
	K_MOUSE3,
	K_MOUSE4,
	K_MOUSE5,
	K_MOUSE6,
	K_MOUSE7,
	K_MOUSE8,

	K_MWHEELDOWN,
	K_MWHEELUP,

	K_LAST_KEY
};

struct sysEvent_t {
	sysEventType_t	evType;
	int				evValue;
	int				evValue2;
	int				evPtrLength;		// bytes of data pointed to by evPtr, for journaling
	void* evPtr;				// this must be manually freed if not NULL

	int				inputDevice;
	bool			IsKeyEvent() const noexcept { return evType == SE_KEY; }
	bool			IsMouseEvent() const noexcept { return evType == SE_MOUSE; }
	bool			IsCharEvent() const noexcept { return evType == SE_CHAR; }
	bool			IsJoystickEvent() const noexcept { return evType == SE_JOYSTICK; }
	bool			IsKeyDown() const noexcept { return evValue2 != 0; }
	keyNum_t		GetKey() const noexcept { return static_cast<keyNum_t>(evValue); }
	int				GetXCoord() const noexcept { return evValue; }
	int				GetYCoord() const noexcept { return evValue2; }
};

void Sys_Error(const char* error, ...);
void Sys_Quit() noexcept;

// will go to the various text consoles
// NOT thread safe - never use in the async paths
void Sys_Printf(const char* msg, ...);

// allow game to yield CPU time
// NOTE: due to SYS_MINSLEEP this is very bad portability karma, and should be completely removed
void Sys_Sleep(int msec) noexcept;

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int Sys_Milliseconds() noexcept;
long long Sys_Microseconds();

long Sys_Time() noexcept;

// for accurate performance testing
double Sys_GetClockTicks() noexcept;
long long Sys_ClockTicksPerSecond() noexcept;

// event generation
sysEvent_t Sys_GetEvent() noexcept;
void Sys_ClearEvents() noexcept;

// input is tied to windows, so it needs to be started up and shut down whenever 
// the main window is recreated
void Sys_InitInput();
void Sys_ShutdownInput() noexcept;

// keyboard input polling
int Sys_PollKeyboardInputEvents() noexcept;
int Sys_ReturnKeyboardInputEvent(const int n, int& ch, bool& state);
void Sys_EndKeyboardInputEvents() noexcept;

std::filesystem::path Sys_DefaultBasePath() noexcept;

// This really isn't the right place to have this, but since this is the 'top level' include
// and has a function signature with 'FILE' in it, it kinda needs to be here =/
using idFileHandle = std::fstream;

char* getLastErrorMsg();

void Sys_DebugMemory_f();

/*
==============================================================

	Networking

==============================================================
*/

enum class netadrtype_t {
	NA_BAD,					// an address lookup failed
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP
};

struct netadr_t {
	netadrtype_t type;
	boost::asio::ip::address address;
	boost::asio::ip::port_type port;
};

#define	PORT_ANY -1

/*
================================================
idUDP
================================================
*/
class idUDP {
public:
	// this just zeros netSocket and port
	idUDP();
	virtual		~idUDP();

	// if the InitForPort fails, the idUDP.port field will remain 0
	bool		InitForPort(int portNumber);

	int			GetPort() const { return bound_to.port; }
	netadr_t	GetAdr() const { return bound_to; }
	//uint32_t	GetUIntAdr() const { return bound_to.address; }
	void		Close();

	bool		GetPacket(netadr_t& from, void* data, int& size, int maxSize);

	void		SendPacket(const netadr_t to, const void* data, int size);

	int			packetsRead;
	int			bytesRead;

	int			packetsWritten;
	int			bytesWritten;

	bool		IsOpen() const { return socket && socket->is_open(); }

private:
	netadr_t bound_to;
	boost::asio::io_context io_context;
	std::unique_ptr<boost::asio::ip::udp::socket> socket;
};

bool Sys_CompareNetAdrBase(const netadr_t a, const netadr_t b);

#endif