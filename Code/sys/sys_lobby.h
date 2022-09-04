
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
	void StartFinding(const idMatchParameters& parms_);
	void Pump();
	void Shutdown(bool retainMigrationInfo = false, bool skipGoodbye = false); // Goto idle state
	void HandlePacket(lobbyAddress_t& remoteAddress, idBitMsg& fragMsg, idPacketProcessor::sessionId_t sessionID);
	lobbyState_t GetState() { return state; }
	bool HasActivePeers() const override;

	static const std::string stateToString[static_cast<int>(lobbyState_t::NUM_STATES)];

	static const int CONNECT_REQUEST_FREQUENCY_IN_SECONDS = 5;		// Frequency at which we resend a request to connect to a server (will increase in frequency over time down to MIN_CONNECT_FREQUENCY_IN_SECONDS)
	static const int MIN_CONNECT_FREQUENCY_IN_SECONDS = 1;		// Min frequency of connection attempts
	static const int MAX_CONNECT_ATTEMPTS = 5;

	static const int OOB_HELLO = 0;
	static const int OOB_GOODBYE = 1;
	static const int OOB_GOODBYE_W_PARTY = 2;
	static const int OOB_GOODBYE_FULL = 3;
	static const int OOB_RESOURCE_LIST = 4;
	static const int OOB_VOICE_AUDIO = 5;

	static const int OOB_MATCH_QUERY = 6;
	static const int OOB_MATCH_QUERY_ACK = 7;

	static const int OOB_SYSTEMLINK_QUERY = 8;

	static const int OOB_MIGRATE_INVITE = 9;

	static const int OOB_BANDWIDTH_TEST = 10;

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
			sessionID = idPacketProcessor::SESSION_ID_INVALID;

			ResetConnectState();
		}

		void ResetConnectState() {
			
		}

		void ResetAllData() {
			ResetConnectState();
			ResetMatchData();
		}

		void ResetMatchData() {
			loaded = false;
			inGame = false;
		}

		bool IsActive() const { return connectionState != connectionState_t::CONNECTION_FREE; }
		bool IsConnected() const { return connectionState == connectionState_t::CONNECTION_ESTABLISHED; }

		connectionState_t GetConnectionState() const;

		connectionState_t connectionState;
		bool loaded; // true if this peer has finished loading the map
		bool inGame; // true if this peer received the first snapshot, and is in-game

		lobbyAddress_t address;

		idPacketProcessor::sessionId_t sessionID;
	};

	std::string GetLobbyName() {
		switch (lobbyType) {
		case lobbyType_t::TYPE_PARTY:		return "TYPE_PARTY";
		case lobbyType_t::TYPE_GAME:			return "TYPE_GAME";
		case lobbyType_t::TYPE_GAME_STATE:	return "TYPE_GAME_STATE";
		}

		return "LOBBY_INVALID";
	}

	int GetNumLobbyUsers() const override { return userList.size(); }
	lobbyUser_t* GetLobbyUser(int index) { return (index >= 0 && index < GetNumLobbyUsers()) ? userList[index] : nullptr; }

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
	void State_Searching();
	void State_Obtaining_Address();
	void State_Connect_Hello_Wait();

	void SetState(lobbyState_t newState);

	void StartCreating();

	int FindPeer(const lobbyAddress_t& remoteAddress, idPacketProcessor::sessionId_t sessionID, bool ignoreSessionID = false);
	int FindFreePeer() const;
	int AddPeer(const lobbyAddress_t& remoteAddress, idPacketProcessor::sessionId_t sessionID);
	void DisconnectPeerFromSession(int p);
	void SetPeerConnectionState(int p, connectionState_t newState, bool skipGoodbye = false);

	void SendGoodbye(const lobbyAddress_t& remoteAddress, bool wasFull = false);
	void QueueReliableMessage(int p, char type, const char* data, int dataLen);

	//void SendConnectionLess(const lobbyAddress_t& remoteAddress, char type) { SendConnectionLess(remoteAddress, type, nullptr, 0); }
	//void SendConnectionLess(const lobbyAddress_t& remoteAddress, char type, const std::byte* data, int dataLen);
	void SendConnectionLess(const lobbyAddress_t& remoteAddress, google::protobuf::Message* proto_msg);
	void SendConnectionRequest();
	void ConnectTo(const lobbyConnectInfo_t& connectInfo, bool fromInvite);
	void HandleGoodbyeFromPeer(int peerNum, lobbyAddress_t& remoteAddress, int msgType);
	void HandleConnectionAttemptFailed();
	bool ConnectToNextSearchResult();
	int HandleInitialPeerConnection(google::protobuf::Message* proto_msg, const lobbyAddress_t& peerAddress, int peerNum);
	void InitStateLobbyHost();

	void SendMembersToLobby(lobbyType_t destLobbyType, const lobbyConnectInfo_t& connectInfo, bool waitForOtherMembers);
	void SendMembersToLobby(idLobby& destLobby, bool waitForOtherMembers);
	void SendPeerMembersToLobby(int peerIndex, lobbyType_t destLobbyType, const lobbyConnectInfo_t& connectInfo, bool waitForOtherMembers);

	void PumpPings();

	void PumpPackets();

	// SessionID helpers
	idPacketProcessor::sessionId_t EncodeSessionID(uint32_t key) const;
	void DecodeSessionID(idPacketProcessor::sessionId_t sessionID, uint32_t& key) const;
	idPacketProcessor::sessionId_t GenerateSessionID() const;
	bool SessionIDCanBeUsedForInBand(idPacketProcessor::sessionId_t sessionID) const;
	idPacketProcessor::sessionId_t IncrementSessionID(idPacketProcessor::sessionId_t sessionID) const;

	//
	// sys_session_instance_users.cpp
	//

	lobbyUser_t* AllocUser(const lobbyUser_t& defaults);
	void FreeUser(lobbyUser_t* user);
	bool VerifyUser(const lobbyUser_t* lobbyUser) const;
	void FreeAllUsers();
	void RegisterUser(lobbyUser_t* lobbyUser);
	void UnregisterUser(lobbyUser_t* lobbyUser);

	// Helper function to create a lobby user from a local user
	lobbyUser_t CreateLobbyUserFromLocalUser();

	// This function is designed to initialize the session users of type lobbyType (TYPE_GAME or TYPE_PARTY)
	// to the current list of local users that are being tracked by the sign-in manager
	void InitSessionUsersFromLocalUsers(bool onlineMatch);

	void RemoveUsersWithDisconnectedPeers();

	//
	// Snapshots
	// sys_session_instance_snapshot.cpp
	//
	void UpdateSnaps();
	void SendSnapshotToPeer(idSnapShot& ss, int p);

	static const int MAX_PEERS = MAX_PLAYERS;

	lobbyType_t lobbyType;
	lobbyState_t state; // State of this lobby
	failedReason_t failedReason;

	int host;						// which peer is the host of this type of session (-1 if we are the host)
	int peerIndexOnHost;			// -1 if we are the host
	lobbyAddress_t hostAddress;				// address of the host for this type of session
	bool isHost;						// true if we are the host
	idLobbyBackend* lobbyBackend = nullptr;

	int helloStartTime;				// Used to determine when the first hello was sent
	int lastConnectRequest;			// Used to determine when the last hello was sent
	int connectionAttempts;			// Number of connection attempts

	idSessionCallbacks* sessionCB;

	std::vector<lobbyConnectInfo_t> searchResults;

	using idLobbyUserList = std::vector<lobbyUser_t*>;
	using idLobbyUserPool = std::vector<lobbyUser_t>;

	idLobbyUserList userList; // list of currently connected users to this lobby
	idLobbyUserList freeUsers; // list of free users
	idLobbyUserPool userPool;

	std::vector<peer_t> peers; // Unique machines connected to this lobby

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

	//virtual	void SendRawPacket(const lobbyAddress_t& to, const void* data, int size, bool useDirectPort) = 0;
	virtual	void SendRawPacket(const lobbyAddress_t& to, boost::asio::streambuf& buf, bool useDirectPort) = 0;

	virtual void GoodbyeFromHost(idLobby& lobby, int peerNum, const lobbyAddress_t& remoteAddress, int msgType) = 0;

	virtual idSession::sessionState_t GetState() const = 0;

	virtual idLobbyBackend* CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual idLobbyBackend* FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual idLobbyBackend* JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) = 0;
	virtual void DestroyLobbyBackend(idLobbyBackend* lobbyBackend) = 0;
};