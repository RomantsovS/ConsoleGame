
#include "sys_lobby_backend.h"

class idSessionCallbacks;

/*
========================
idLobby
========================
*/
class idLobby : public idLobbyBase {
public:
	idLobby();

	enum class lobbyType_t {
		TYPE_PARTY = 0,
		TYPE_GAME = 1,
		TYPE_GAME_STATE = 2,
		TYPE_INVALID = 0xff
	};

	enum class lobbyState_t {
		STATE_IDLE,
		STATE_CREATE_LOBBY_BACKEND,
		STATE_SEARCHING,
		STATE_OBTAINING_ADDRESS,
		STATE_CONNECT_HELLO_WAIT,
		STATE_FINALIZE_CONNECT,
		STATE_FAILED,
		NUM_STATES
	};

	enum class failedReason_t {
		FAILED_UNKNOWN,
		FAILED_CONNECT_FAILED,
		FAILED_MIGRATION_CONNECT_FAILED,
	};

	void Initialize(lobbyType_t sessionType_, idSessionCallbacks* callbacks);
	void StartHosting(const idMatchParameters& parms);
	void Pump();
	void Shutdown(bool retainMigrationInfo = false, bool skipGoodbye = false); // Goto idle state
	lobbyState_t GetState() { return state; }
	bool HasActivePeers() const override;

	static const std::string stateToString[static_cast<int>(lobbyState_t::NUM_STATES)];

	enum class connectionState_t {
		CONNECTION_FREE = 0,		// Free peer slot
		CONNECTION_CONNECTING = 1,		// Waiting for response from host for initial connection
		CONNECTION_ESTABLISHED = 2,		// Connection is established and active
	};

	struct peer_t {
		peer_t() {
			loaded = false;
			inGame = false;
			connectionState = connectionState_t::CONNECTION_FREE;
		}

		bool IsConnected() const { return connectionState == connectionState_t::CONNECTION_ESTABLISHED; }

		connectionState_t GetConnectionState() const;

		connectionState_t connectionState;
		bool loaded; // true if this peer has finished loading the map
		bool inGame; // true if this peer received the first snapshot, and is in-game		
	};

	const std::string GetLobbyName() {
		switch (lobbyType) {
		case lobbyType_t::TYPE_PARTY:		return "TYPE_PARTY";
		case lobbyType_t::TYPE_GAME:			return "TYPE_GAME";
		case lobbyType_t::TYPE_GAME_STATE:	return "TYPE_GAME_STATE";
		}

		return "LOBBY_INVALID";
	}

	int GetNumLobbyUsers() const override { return userList.size(); }

	const idMatchParameters& GetMatchParms() const override { return parms; }

	lobbyType_t GetActingGameStateLobbyType() const;

	// If IsHost is true, we are a host accepting connections from peers
	bool IsHost() const override { return isHost; }
	// If IsPeer is true, we are a peer, with an active connection to a host
	bool IsPeer() const override {
		if (host == -1) {
			return false;		// Can't possibly be a peer if we haven't setup a host
		}
		assert(!IsHost());
		return peers[host].IsConnected();
	}

	// IsRunningAsHostOrPeer means we are either an active host, and can accept connections from peers, or we are a peer with an active connection to a host
	bool IsRunningAsHostOrPeer() const { return IsHost() || IsPeer(); }
	bool IsLobbyActive() const { return IsRunningAsHostOrPeer(); }

	// State functions
	void State_Idle();
	void State_Create_Lobby_Backend();

	void SetState(lobbyState_t newState);

	void StartCreating();

	void HandleConnectionAttemptFailed();
	void InitStateLobbyHost();

	void PumpPings();

	void PumpPackets();

	//
	// Snapshots
	// sys_session_instance_snapshot.cpp
	//
	void UpdateSnaps();

	static const int MAX_PEERS = MAX_PLAYERS;

	lobbyType_t lobbyType;
	lobbyState_t state; // State of this lobby
	failedReason_t failedReason;

	int host;						// which peer is the host of this type of session (-1 if we are the host)
	int peerIndexOnHost;			// -1 if we are the host
	lobbyAddress_t hostAddress;				// address of the host for this type of session
	bool isHost;						// true if we are the host
	idLobbyBackend* lobbyBackend = nullptr;

	idSessionCallbacks* sessionCB;

	using idLobbyUserList = std::vector<lobbyUser_t*>;

	idLobbyUserList userList; // list of currently connected users to this lobby

	std::array<peer_t, MAX_PEERS> peers; // Unique machines connected to this lobby

	idMatchParameters parms;

	bool loaded; // Used for game sessions, whether this machine is loaded or not
};

/*
========================
idSessionCallbacks
========================
*/
class idSessionCallbacks {
public:
	~idSessionCallbacks() = default;
	virtual idLobby& GetPartyLobby() = 0;
	virtual idLobby& GetGameLobby() = 0;
	virtual idLobby& GetActingGameStateLobby() = 0;

	virtual idLobbyBackend* CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual void DestroyLobbyBackend(idLobbyBackend* lobbyBackend) = 0;
};