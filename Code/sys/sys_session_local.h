#ifndef SYS_SYS_SESSION_LOCAL_H_
#define SYS_SYS_SESSION_LOCAL_H_

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
	virtual ~idSessionLocal() = default;
	idSessionLocal(const idSessionLocal&) = default;
	idSessionLocal& operator=(const idSessionLocal&) = default;
	idSessionLocal(idSessionLocal&&) = default;
	idSessionLocal& operator=(idSessionLocal&&) = default;

	void InitBaseState() noexcept;

	// Lobby management
	void CreateMatch(const idMatchParameters& parms_) override;

	void StartMatch() noexcept override;
	void MoveToPressStart() override;
	void FinishDisconnect() noexcept override;

	const idMatchParameters& GetMatchParms() const noexcept override  { return parms; }

	// Misc
	void QuitMatchToTitle() noexcept override; // Will forcefully quit the match and return to the title screen.
	void LoadingFinished() noexcept override;

	sessionState_t	GetState() const override;

	void UpdateSignInManager() override;
	void RegisterLocalUser() noexcept override { localUserRegistered = true; }
	bool IsLocalUserRegistered() noexcept override { return localUserRegistered; }

	void Initialize() noexcept override = 0;
	void Shutdown() noexcept override = 0;
protected:
	//=====================================================================================================
	// Common functions (sys_session_local.cpp)
	//=====================================================================================================

	void SetState(state_t newState) noexcept;
protected:
	state_t localState;

	void MoveToMainMenu() noexcept; // End all session (async), and return to IDLE state
private:
	bool localUserRegistered;
	idMatchParameters parms;

	void StartLoading() noexcept;
};

#endif
