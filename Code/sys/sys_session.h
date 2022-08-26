#ifndef SYS_SYS_SESSION_H_ 
#define SYS_SYS_SESSION_H_

static const int MAX_PLAYERS = 8;

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
idLobbyBase
================================================
*/
class idLobbyBase {
public:
	// General lobby functionality
	virtual bool IsHost() const = 0;
	virtual bool IsPeer() const = 0;
	virtual bool HasActivePeers() const = 0;
	virtual int GetNumLobbyUsers() const = 0;

	virtual const idMatchParameters& GetMatchParms() const = 0;
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
	virtual void CreatePartyLobby(const idMatchParameters& parms_) = 0;
	virtual void CreateMatch(const idMatchParameters& parms_) = 0;
	virtual void StartMatch() = 0;
	virtual void EndMatch(bool premature = false) = 0;	// Meant for host to end match gracefully, go back to lobby, tally scores, etc
	virtual void QuitMatchToTitle() = 0; // Will forcefully quit the match and return to the title screen.
	virtual sessionState_t GetBackState() = 0;
	virtual void Cancel() = 0;
	virtual void MoveToPressStart() = 0;
	virtual void FinishDisconnect() = 0;
	virtual void LoadingFinished() = 0;

	virtual void Pump() = 0;

	// Lobby user/peer access
	// The party and game lobby are the two platform lobbies that notify the backends (Steam/PSN/LIVE of changes)
	virtual idLobbyBase& GetPartyLobbyBase() = 0;
	virtual idLobbyBase& GetGameLobbyBase() = 0;

	// Game state lobby is the lobby used while in-game.  It is so the dedicated server can host this lobby
	// and have all platform clients join. It does NOT notify the backends of changes, it's purely for the dedicated
	// server to be able to host the in-game lobby.
	virtual idLobbyBase& GetActingGameStateLobbyBase() = 0;

	virtual sessionState_t GetState() const = 0;

	virtual void UpdateSignInManager() = 0;

	virtual void RegisterLocalUser() = 0;
	virtual bool IsLocalUserRegistered() = 0;
};

extern idSession* session;

#endif