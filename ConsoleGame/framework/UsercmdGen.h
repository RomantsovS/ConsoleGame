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
const int IMPULSE_0 = 0;			// weap 0
const int IMPULSE_1 = 1;			// weap 1
const int IMPULSE_2 = 2;			// weap 2
const int IMPULSE_3 = 3;			// weap 3
const int IMPULSE_4 = 4;			// weap 4
const int IMPULSE_5 = 5;			// weap 5
const int IMPULSE_6 = 6;			// weap 6
const int IMPULSE_7 = 7;			// weap 7
const int IMPULSE_8 = 8;			// weap 8
const int IMPULSE_9 = 9;			// weap 9
const int IMPULSE_10 = 10;			// weap 10
const int IMPULSE_11 = 11;			// weap 11
const int IMPULSE_12 = 12;			// weap 12
const int IMPULSE_13 = 13;			// weap reload
const int IMPULSE_14 = 14;			// weap next
const int IMPULSE_15 = 15;			// weap prev
const int IMPULSE_16 = 16;			// toggle flashlight on/off
const int IMPULSE_18 = 18;			// center view
const int IMPULSE_19 = 19;			// show PDA/SCORES
const int IMPULSE_22 = 22;			// spectate
const int IMPULSE_25 = 25;			// Envirosuit light
const int IMPULSE_27 = 27;			// Chainsaw
const int IMPULSE_28 = 28;			// quick 0
const int IMPULSE_29 = 29;			// quick 1
const int IMPULSE_30 = 30;			// quick 2
const int IMPULSE_31 = 31;			// quick 3

class usercmd_t {
public:
	usercmd_t() :
		forwardmove(),
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
	//short		angles[3];						// view angles
	signed char	forwardmove;					// forward/backward movement
	signed char	rightmove;						// left/right movement
	
	unsigned char buttons;						// buttons
	/*int			clientGameMilliseconds;			// time this usercmd was sent from the client
	int			serverGameMilliseconds;			// interpolated server time this was applied on
	uint16		fireCount;						// number of times we've fired
*/
	// Not syncronized
	unsigned char impulse;						// impulse command
	/*byte		impulseSequence;				// incremented every time there's a new impulse

	short		mx;								// mouse delta x
	short		my;								// mouse delta y

	// Clients are authoritative on their positions
	idVec3		pos;
	float		speedSquared;*/

public:
	/*void		Serialize(class idSerializer& s, const usercmd_t& base);
	void		ByteSwap();						// on big endian systems, byte swap the shorts and ints
	bool		operator==(const usercmd_t& rhs) const;*/
};

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
	usercmdButton_t	button;
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
	virtual	void		Init() = 0;

	// Prepares for a new map.
	virtual void		InitForNewMap() = 0;

	// Shut down.
	virtual void		Shutdown() = 0;

	// Clears all key states and face straight.
	virtual	void		Clear() = 0;

	// Clears view angles.
	/*virtual void		ClearAngles() = 0;

	// When the console is down or the menu is up, only emit default usercmd, so the player isn't moving around.
	// Each subsystem (session and game) may want an inhibit will OR the requests.
	virtual void		InhibitUsercmd(inhibit_t subsystem, bool inhibit) = 0;
	*/
	// Set a value that can safely be referenced by UsercmdInterrupt() for each key binding.
	virtual	int			CommandStringUsercmdData(const std::string& cmdString) = 0;

	// Continuously modified, never reset. For full screen guis.
	//virtual void		MouseState(int* x, int* y, int* button, bool* down) = 0;

	// Directly sample a button.
	virtual int			ButtonState(int key) = 0;

	// Directly sample a keystate.
	virtual int			KeyState(int key) = 0;

	// called at vsync time
	virtual void		BuildCurrentUsercmd(int deviceNum) = 0;

	// return the current usercmd
	virtual usercmd_t	GetCurrentUsercmd() = 0;
};

extern idUsercmdGen* usercmdGen;
//extern userCmdString_t	userCmdStrings[];

#endif

