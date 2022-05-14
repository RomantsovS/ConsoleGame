#ifndef SYS_SYS_SESSION_H_ 
#define SYS_SYS_SESSION_H_

/*
================================================
idMatchParameters
================================================
*/
class idMatchParameters {
public:
	idMatchParameters() {}

	std::string mapName; // This is only used for SP (gameMap == GAME_MAP_SINGLEPLAYER)
};

/*
================================================
idSession
================================================
*/
class idSession {
public:

	enum class sessionState_t {
		PRESS_START,
		IDLE,
		SEARCHING,
		CONNECTING,
		PARTY_LOBBY,
		GAME_LOBBY,
		LOADING,
		INGAME,
		BUSY,
		MAX_STATES
	};

	idSession() = default;
	virtual ~idSession() {}
	idSession(const idSession&) = default;
	idSession& operator=(const idSession&) = default;
	idSession(idSession&&) = default;
	idSession& operator=(idSession&&) = default;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;

	//=====================================================================================================
	// Lobby management 
	//=====================================================================================================
	virtual void CreateMatch(const idMatchParameters& parms_) = 0;
	virtual void StartMatch() = 0;
	virtual void QuitMatchToTitle() = 0; // Will forcefully quit the match and return to the title screen.
	virtual void MoveToPressStart() = 0;
	virtual void FinishDisconnect() = 0;
	virtual void LoadingFinished() = 0;

	virtual const idMatchParameters& GetMatchParms() const = 0;

	virtual sessionState_t	GetState() const = 0;

	virtual void UpdateSignInManager() = 0;

	virtual void RegisterLocalUser() = 0;
	virtual bool IsLocalUserRegistered() = 0;
};

extern idSession* session;

#endif