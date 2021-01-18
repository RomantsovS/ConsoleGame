#include "../idlib/precompiled.h"
#pragma hdrstop

const int KEY_MOVESPEED = 127;

std::vector<userCmdString_t> userCmdStrings = {
	{ "_moveUp",		UB_MOVEUP },
	{ "_moveDown",		UB_MOVEDOWN },
	{ "_left",			UB_LOOKLEFT },
	{ "_right",			UB_LOOKRIGHT },
	{ "_forward",		UB_MOVEFORWARD },
	{ "_back",			UB_MOVEBACK },
	{ "_lookUp",		UB_LOOKUP },
	{ "_lookDown",		UB_LOOKDOWN },
	{ "_moveLeft",		UB_MOVELEFT },
	{ "_moveRight",		UB_MOVERIGHT },

	{ "_attack",		UB_ATTACK },
	{ "_speed",			UB_SPEED },
	{ "_zoom",			UB_ZOOM },
	{ "_showScores",	UB_SHOWSCORES },
	{ "_use",			UB_USE },

	{ "_impulse0",		UB_IMPULSE0 },
	{ "_impulse1",		UB_IMPULSE1 },
	{ "_impulse2",		UB_IMPULSE2 },
	{ "_impulse3",		UB_IMPULSE3 },
	{ "_impulse4",		UB_IMPULSE4 },
	{ "_impulse5",		UB_IMPULSE5 },
	{ "_impulse6",		UB_IMPULSE6 },
	{ "_impulse7",		UB_IMPULSE7 },
	{ "_impulse8",		UB_IMPULSE8 },
	{ "_impulse9",		UB_IMPULSE9 },
	{ "_impulse10",		UB_IMPULSE10 },
	{ "_impulse11",		UB_IMPULSE11 },
	{ "_impulse12",		UB_IMPULSE12 },
	{ "_impulse13",		UB_IMPULSE13 },
	{ "_impulse14",		UB_IMPULSE14 },
	{ "_impulse15",		UB_IMPULSE15 },
	{ "_impulse16",		UB_IMPULSE16 },
	{ "_impulse17",		UB_IMPULSE17 },
	{ "_impulse18",		UB_IMPULSE18 },
	{ "_impulse19",		UB_IMPULSE19 },
	{ "_impulse20",		UB_IMPULSE20 },
	{ "_impulse21",		UB_IMPULSE21 },
	{ "_impulse22",		UB_IMPULSE22 },
	{ "_impulse23",		UB_IMPULSE23 },
	{ "_impulse24",		UB_IMPULSE24 },
	{ "_impulse25",		UB_IMPULSE25 },
	{ "_impulse26",		UB_IMPULSE26 },
	{ "_impulse27",		UB_IMPULSE27 },
	{ "_impulse28",		UB_IMPULSE28 },
	{ "_impulse29",		UB_IMPULSE29 },
	{ "_impulse30",		UB_IMPULSE30 },
	{ "_impulse31",		UB_IMPULSE31 },

	{ "",				UB_NONE }
};

class idUsercmdGenLocal : public idUsercmdGen {
public:
	idUsercmdGenLocal();

	void			Init() override;

	void			InitForNewMap() override;

	void			Shutdown() override;

	void			Clear() override;

	/*void			ClearAngles();

	void			InhibitUsercmd(inhibit_t subsystem, bool inhibit);
	*/
	int				CommandStringUsercmdData(const std::string& cmdString);

	void			BuildCurrentUsercmd(int deviceNum) override;

	usercmd_t		GetCurrentUsercmd() override { return cmd; };

	//void			MouseState(int* x, int* y, int* button, bool* down);

	int				ButtonState(int key) override;
	int				KeyState(int key) override;

private:
	void			MakeCurrent();
	void			InitCurrent();

	/*bool			Inhibited();
	void			AdjustAngles();*/
	void			KeyMove();
	/*void			CircleToSquare(float& axis_x, float& axis_y) const;
	void			HandleJoystickAxis(int keyNum, float unclampedValue, float threshold, bool positive);
	void			JoystickMove();
	void			JoystickMove2();
	void			MouseMove();*/
	void			CmdButtons();

	/*void			AimAssist();

	void			Mouse();*/
	void			Keyboard();
	//void			Joystick(int deviceNum);

	void			Key(int keyNum, bool down);

	/*idVec3			viewangles;
	int				impulseSequence;*/
	int				impulse;
	/*
	buttonState_t	toggled_crouch;
	buttonState_t	toggled_run;
	buttonState_t	toggled_zoom;*/

	int				buttonState[UB_MAX_BUTTONS];
	bool			keyState[static_cast<int>(keyNum_t::K_LAST_KEY)];

	//int				inhibitCommands;	// true when in console or menu locally

	bool			initialized;

	usercmd_t		cmd;		// the current cmd being built

	/*int				continuousMouseX, continuousMouseY;	// for gui event generatioin, never zerod
	int				mouseButton;						// for gui event generatioin
	bool			mouseDown;

	int				mouseDx, mouseDy;	// added to by mouse events
	float			joystickAxis[MAX_JOYSTICK_AXIS];	// set by joystick events
	
	int				pollTime;
	int				lastPollTime;
	float			lastLookValuePitch;
	float			lastLookValueYaw;

	static idCVar	in_yawSpeed;
	static idCVar	in_pitchSpeed;
	static idCVar	in_angleSpeedKey;
	static idCVar	in_toggleRun;
	static idCVar	in_toggleCrouch;
	static idCVar	in_toggleZoom;
	static idCVar	sensitivity;
	static idCVar	m_pitch;
	static idCVar	m_yaw;
	static idCVar	m_smooth;
	static idCVar	m_showMouseRate;*/
};

static idUsercmdGenLocal localUsercmdGen;
idUsercmdGen* usercmdGen = &localUsercmdGen;

/*
================
idUsercmdGenLocal::idUsercmdGenLocal
================
*/
idUsercmdGenLocal::idUsercmdGenLocal() {
	initialized = false;

	impulse = 0;

	Clear();
}

/*
===============
idUsercmdGenLocal::ButtonState

Returns (the fraction of the frame) that the key was down
===============
*/
int	idUsercmdGenLocal::ButtonState(int key) {
	if (key < 0 || key >= UB_MAX_BUTTONS) {
		return -1;
	}
	return (buttonState[key] > 0) ? 1 : 0;
}

/*
===============
idUsercmdGenLocal::KeyState

Returns (the fraction of the frame) that the key was down
bk20060111
===============
*/
int	idUsercmdGenLocal::KeyState(int key) {
	if (key < 0 || key >= static_cast<int>(keyNum_t::K_LAST_KEY)) {
		return -1;
	}
	return (keyState[key]) ? 1 : 0;
}

/*
================
idUsercmdGenLocal::KeyMove

Sets the usercmd_t based on key states
================
*/
void idUsercmdGenLocal::KeyMove() {
	int forward = 0;
	int side = 0;

	side += KEY_MOVESPEED * ButtonState(UB_MOVERIGHT);
	side -= KEY_MOVESPEED * ButtonState(UB_MOVELEFT);

	forward -= KEY_MOVESPEED * ButtonState(UB_MOVEUP);
	forward += KEY_MOVESPEED * ButtonState(UB_MOVEDOWN);

	cmd.forwardmove += idMath::ClampChar(forward);
	cmd.rightmove += idMath::ClampChar(side);
}

/*
==============
idUsercmdGenLocal::CmdButtons
==============
*/
void idUsercmdGenLocal::CmdButtons() {
	cmd.buttons = 0;

	// check the attack button
	if (ButtonState(UB_ATTACK)) {
		cmd.buttons |= BUTTON_ATTACK;
	}

	// check the use button
	if (ButtonState(UB_USE)) {
		cmd.buttons |= BUTTON_USE;
	}
}

/*
================
idUsercmdGenLocal::InitCurrent

inits the current command for this frame
================
*/
void idUsercmdGenLocal::InitCurrent() {
	memset(&cmd, 0, sizeof(cmd));
	//cmd.impulseSequence = impulseSequence;
	cmd.impulse = impulse;
	//cmd.buttons |= (in_alwaysRun.GetBool() && common->IsMultiplayer()) ? BUTTON_RUN : 0;
}

/*
================
idUsercmdGenLocal::MakeCurrent

creates the current command for this frame
================
*/
void idUsercmdGenLocal::MakeCurrent() {
	// set button bits
	CmdButtons();

	// get basic movement from keyboard
	KeyMove();

	impulse = cmd.impulse;
}

/*
================
idUsercmdGenLocal::CommandStringUsercmdData

Returns the button if the command string is used by the usercmd generator.
================
*/
int	idUsercmdGenLocal::CommandStringUsercmdData(const std::string& cmdString) {
	auto iter = std::find_if(userCmdStrings.begin(), userCmdStrings.end(), [&cmdString](auto& ucs) {return cmdString.compare(ucs.string) == 0; });

	if (iter != userCmdStrings.end())
		return iter->button;

	return UB_NONE;
}

/*
================
idUsercmdGenLocal::Init
================
*/
void idUsercmdGenLocal::Init() {
	initialized = true;
}

/*
================
idUsercmdGenLocal::InitForNewMap
================
*/
void idUsercmdGenLocal::InitForNewMap() {
	impulse = 0;
	Clear();
}

/*
================
idUsercmdGenLocal::Shutdown
================
*/
void idUsercmdGenLocal::Shutdown() {
	initialized = false;
}

/*
================
idUsercmdGenLocal::Clear
================
*/
void idUsercmdGenLocal::Clear() {
	// clears all key states 
	memset(buttonState, 0, sizeof(buttonState));
	memset(keyState, false, sizeof(keyState));
}

/*
===================
idUsercmdGenLocal::Key

Handles mouse/keyboard button actions
===================
*/
void idUsercmdGenLocal::Key(int keyNum, bool down) {

	// Sanity check, sometimes we get double message :(
	if (keyState[keyNum] == down) {
		return;
	}
	keyState[keyNum] = down;

	int action = idKeyInput::GetUsercmdAction(keyNum);

	if (down) {
		buttonState[action]++;
		//if (!Inhibited()) {
			if (action >= UB_IMPULSE0 && action <= UB_IMPULSE31) {
				cmd.impulse = action - UB_IMPULSE0;
				//cmd.impulseSequence++;
			}
		//}
	}
	else {
		buttonState[action]--;
		// we might have one held down across an app active transition
		if (buttonState[action] < 0) {
			buttonState[action] = 0;
		}
	}
}

/*
===============
idUsercmdGenLocal::Keyboard
===============
*/
void idUsercmdGenLocal::Keyboard() {
	int numEvents = Sys_PollKeyboardInputEvents();

	// Study each of the buffer elements and process them.
	for (int i = 0; i < numEvents; i++) {
		int key;
		bool state;
		if (Sys_ReturnKeyboardInputEvent(i, key, state)) {
			Key(key, state);
		}
	}

	Sys_EndKeyboardInputEvents();
}

void Sys_EndKeyboardInputEvents() {
}

/*
================
idUsercmdGenLocal::BuildCurrentUsercmd
================
*/
void idUsercmdGenLocal::BuildCurrentUsercmd(int deviceNum) {
	// initialize current usercmd
	InitCurrent();

	// process the system mouse events
	//Mouse();

	// process the system keyboard events
	Keyboard();

	// create the usercmd
	MakeCurrent();
}