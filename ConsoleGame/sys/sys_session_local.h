#ifndef SYS_SYS_SESSION_LOCAL_H_
#define SYS_SYS_SESSION_LOCAL_H_

#include "sys_session.h"

/*
================================================
idSessionLocal
================================================
*/
class idSessionLocal : public idSession {
protected:
	// Overall state of the session
	enum class state_t {
		STATE_PRESS_START,							// We are at press start
		STATE_IDLE,									// We are at the main menu
		STATE_PARTY_LOBBY_HOST,						// We are in the party lobby menu as host
		STATE_PARTY_LOBBY_PEER,						// We are in the party lobby menu as a peer
		STATE_GAME_LOBBY_HOST,						// We are in the game lobby as a host
		STATE_GAME_LOBBY_PEER,						// We are in the game lobby as a peer
		STATE_GAME_STATE_LOBBY_HOST,				// We are in the game state lobby as a host
		STATE_GAME_STATE_LOBBY_PEER,				// We are in the game state lobby as a peer
		STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY,		// We are creating a party lobby, and will move to that state when done
		STATE_CREATE_AND_MOVE_TO_GAME_LOBBY,		// We are creating a game lobby, and will move to that state when done
		STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY,	// We are creating a game state lobby, and will move to that state when done
		STATE_FIND_OR_CREATE_MATCH,
		STATE_CONNECT_AND_MOVE_TO_PARTY,
		STATE_CONNECT_AND_MOVE_TO_GAME,
		STATE_CONNECT_AND_MOVE_TO_GAME_STATE,
		STATE_BUSY,									// Doing something internally like a QoS/bandwidth challenge

		// These are last, so >= STATE_LOADING tests work
		STATE_LOADING,								// We are loading the map, preparing to go into a match
		STATE_INGAME,								// We are currently in a match
		NUM_STATES
	};

public:
	idSessionLocal();
	virtual ~idSessionLocal();

	void InitBaseState();

	// Misc
	virtual void LoadingFinished() override;

	virtual sessionState_t	GetState() const override;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
protected:
	//=====================================================================================================
	// Common functions (sys_session_local.cpp)
	//=====================================================================================================

	void SetState(state_t newState);
protected:
	state_t localState;
};

#endif
