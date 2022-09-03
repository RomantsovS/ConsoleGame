#ifndef SYS_SYS_SESSION_LOCAL_H_
#define SYS_SYS_SESSION_LOCAL_H_

#include "sys_lobby_backend.h"
#include "sys_lobby.h"

/*
================================================
idSessionLocal
================================================
*/
class idSessionLocal : public idSession {
	friend class idSessionLocalCallbacks;
	friend class idNetSessionPort;
	friend class lobbyAddress_t;
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
	void CreatePartyLobby(const idMatchParameters& parms_) override;
	void FindOrCreateMatch(const idMatchParameters& parms) override;
	void CreateMatch(const idMatchParameters& parms_) override;

	void StartMatch() noexcept override;
	sessionState_t GetBackState() override;
	void Cancel() override;
	void MoveToPressStart() override;
	void FinishDisconnect() noexcept override;

	// Misc
	void EndMatch(bool premature = false) override; // Meant for host to end match gracefully, go back to lobby, tally scores, etc
	void QuitMatchToTitle() noexcept override; // Will forcefully quit the match and return to the title screen.
	void LoadingFinished() noexcept override;
	void Pump() override;

	sessionState_t GetState() const override;

	//void SendUsercmds(idBitMsg& msg);
	void SendSnapshot(idSnapShot& ss) override;

	void UpdateSignInManager() override;
	void RegisterLocalUser() noexcept override { localUserRegistered = true; }
	bool IsLocalUserRegistered() noexcept override { return localUserRegistered; }

	void Initialize() noexcept override = 0;
	void Shutdown() noexcept override = 0;
protected:
	//=====================================================================================================
	// Common functions (sys_session_local.cpp)
	//=====================================================================================================

	bool HandleState();

	// The party and game lobby are the two platform lobbies that notify the backends (Steam/PSN/LIVE of changes)
	idLobby& GetPartyLobby() { return partyLobby; }
	const idLobby& GetPartyLobby() const { return partyLobby; }
	idLobby& GetGameLobby() { return gameLobby; }
	const idLobby& GetGameLobby() const { return gameLobby; }

	// Game state lobby is the lobby used while in-game.  It is so the dedicated server can host this lobby
	// and have all platform clients join. It does NOT notify the backends of changes, it's purely for the dedicated
	// server to be able to host the in-game lobby.
	// Generally, you would call GetActingGameStateLobby.  If we are not using game state lobby, GetActingGameStateLobby will return GetGameLobby insread.
	idLobby& GetGameStateLobby() { return gameStateLobby; }
	const idLobby& GetGameStateLobby() const { return gameStateLobby; }

	idLobby& GetActingGameStateLobby();

	// GetActivePlatformLobby will return either the game or party lobby, it won't return the game state lobby
	// This function is generally used for menus, in-game code should refer to GetActingGameStateLobby
	idLobby* GetActivePlatformLobby();

	idLobbyBase& GetPartyLobbyBase() override { return partyLobby; }
	idLobbyBase& GetGameLobbyBase() override { return gameLobby; }
	idLobbyBase& GetActingGameStateLobbyBase() override { return GetActingGameStateLobby(); }

	idLobbyBase& GetActivePlatformLobbyBase() override;

	void SetState(state_t newState) noexcept;
	bool HandlePackets();
protected:
	static const std::string stateToString[static_cast<int>(state_t::NUM_STATES)];

	state_t localState;

	//connectType_t connectType;
	int connectTime;

	idLobby partyLobby;
	idLobby gameLobby;
	idLobby gameStateLobby;
	idLobby stubLobby; // We use this when we request the active lobby when we are not in a lobby (i.e at press start)

	//------------------------
	// State functions
	//------------------------
	bool State_Party_Lobby_Host();
	//bool State_Party_Lobby_Peer();
	bool State_Game_Lobby_Host();
	/*bool State_Game_Lobby_Peer();
	bool State_Game_State_Lobby_Host();
	bool State_Game_State_Lobby_Peer();*/
	bool State_Loading();
	bool State_InGame();
	bool State_Find_Or_Create_Match();
	bool State_Create_And_Move_To_Party_Lobby();
	bool State_Create_And_Move_To_Game_Lobby();

	bool State_Connect_And_Move_To_Game();

	void SendRawPacket(const lobbyAddress_t& to, const void* data, int size, bool dedicated);
	void SendRawPacket(const lobbyAddress_t& to, boost::asio::streambuf& buf , bool dedicated);
	bool ReadRawPacket(lobbyAddress_t& from, boost::asio::streambuf::mutable_buffers_type& bufs, int& size, int maxSize);

	virtual idNetSessionPort& GetPort(bool dedicated = false) = 0;
	virtual idLobbyBackend* CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual idLobbyBackend* FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual idLobbyBackend* JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual void DestroyLobbyBackend(idLobbyBackend* lobby) = 0;
	virtual void PumpLobbies() = 0;

	void EndMatchInternal(bool premature = false);

	void MoveToMainMenu() noexcept; // End all session (async), and return to IDLE state
	bool WaitOnLobbyCreate(idLobby& lobby);
	bool DetectDisconnectFromService(bool cancelAndShowMsg);
	void HandleConnectionFailed(idLobby& lobby, bool wasFull);
	bool HandleConnectAndMoveToLobby(idLobby& lobby);

	void ValidateLobbies();
	void ValidateLobby(idLobby& lobby);
private:
	bool localUserRegistered;

protected:

	void StartLoading() noexcept;

	std::unique_ptr<idSessionCallbacks> sessionCallbacks;
};

/*
========================
idSessionLocalCallbacks
	The more the idLobby class needs to call back into this class, the more likely we're doing something wrong, and there is a better way.
========================
*/
class idSessionLocalCallbacks : public idSessionCallbacks {
public:
	idSessionLocalCallbacks(idSessionLocal* sessionLocal_) { sessionLocal = sessionLocal_; }

	idLobby& GetPartyLobby() override { return sessionLocal->GetPartyLobby(); }
	idLobby& GetGameLobby() override { return sessionLocal->GetGameLobby(); }
	idLobby& GetActingGameStateLobby() override { return sessionLocal->GetActingGameStateLobby(); }

	void SendRawPacket(const lobbyAddress_t& to, const void* data, int size, bool useDirectPort) override { sessionLocal->SendRawPacket(to, data, size, useDirectPort); }
	void SendRawPacket(const lobbyAddress_t& to, boost::asio::streambuf& buf, bool useDirectPort) override {
		sessionLocal->SendRawPacket(to, buf, useDirectPort);
	}

	idSession::sessionState_t GetState() const override { return sessionLocal->GetState(); }

	idLobbyBackend* CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) override;
	idLobbyBackend* FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) override;
	idLobbyBackend* JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) override;
	void DestroyLobbyBackend(idLobbyBackend* lobby) override;

	idSessionLocal* sessionLocal;
};

#endif
