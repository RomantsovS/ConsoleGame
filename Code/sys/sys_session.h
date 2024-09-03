#ifndef SYS_SYS_SESSION_H_ 
#define SYS_SYS_SESSION_H_

static const int MAX_PLAYERS = 8;

enum class matchFlags_t {
	MATCH_STATS = BIT(0),		// Match will upload leaderboard/achievement scores
	MATCH_ONLINE = BIT(1),		// Match will require users to be online
	MATCH_RANKED = BIT(2),		// Match will affect rank
	MATCH_PRIVATE = BIT(3),		// Match will NOT be searchable through FindOrCreateMatch
	MATCH_INVITE_ONLY = BIT(4),		// Match visible through invite only

	MATCH_REQUIRE_PARTY_LOBBY = BIT(5),		// This session uses a party lobby
	MATCH_PARTY_INVITE_PLACEHOLDER = BIT(6),		// Party is never shown in the UI, it's simply used as a placeholder for invites
	MATCH_JOIN_IN_PROGRESS = BIT(7),		// Join in progress supported for this match
};

inline bool MatchTypeIsOnline(uint8_t matchFlags) { return (matchFlags & static_cast<int>(matchFlags_t::MATCH_ONLINE)) ? true : false; }

const int8_t GAME_MAP_RANDOM = -1;

const int DefaultPartyFlags = static_cast<int>(matchFlags_t::MATCH_JOIN_IN_PROGRESS) | static_cast<int>(matchFlags_t::MATCH_ONLINE);
const int DefaultPublicGameFlags = static_cast<int>(matchFlags_t::MATCH_JOIN_IN_PROGRESS) | static_cast<int>(matchFlags_t::MATCH_REQUIRE_PARTY_LOBBY)
| static_cast<int>(matchFlags_t::MATCH_RANKED) | static_cast<int>(matchFlags_t::MATCH_STATS);
const int DefaultPrivateGameFlags = static_cast<int>(matchFlags_t::MATCH_JOIN_IN_PROGRESS) | static_cast<int>(matchFlags_t::MATCH_REQUIRE_PARTY_LOBBY)
| static_cast<int>(matchFlags_t::MATCH_PRIVATE);

/*
================================================
idMatchParameters
================================================
*/
class idMatchParameters {
public:
	idMatchParameters() :
		gameMap(GAME_MAP_RANDOM),
		matchFlags(0) {}

	int8_t gameMap;
	uint8_t matchFlags;

	std::string mapName; // This is only used for SP (gameMap == GAME_MAP_SINGLEPLAYER)
};

struct lobbyUserID_t {
public:

	lobbyUserID_t() : lobbyType{ 0xFF } {}

	explicit lobbyUserID_t(uint32_t localUser_, std::byte lobbyType_) : localUserHandle(localUser_), lobbyType(lobbyType_) {}
	
	bool operator == (const lobbyUserID_t& other) const {
		return localUserHandle == other.localUserHandle && lobbyType == other.lobbyType;		// Lobby type must match
	}

	bool operator != (const lobbyUserID_t& other) const {
		return !(*this == other);
	}

	bool operator < (const lobbyUserID_t& other) const {
		if (localUserHandle == other.localUserHandle) {
			return lobbyType < other.lobbyType;		// Lobby type tie breaker
		}

		return localUserHandle < other.localUserHandle;
	}
	/*
	bool CompareIgnoreLobbyType(const lobbyUserID_t& other) const {
		return localUserHandle == other.localUserHandle;
	}
	*/
	uint32_t GetLocalUserHandle() const { return localUserHandle; }
	std::byte GetLobbyType() const { return lobbyType; }
	
	bool IsValid() const { return localUserHandle > 0 && lobbyType != std::byte{ 0xFF }; }
	
	void WriteToMsg(idBitMsg& msg) {
		msg.WriteLong(localUserHandle);
		msg.WriteBytes(static_cast<int32_t>(lobbyType), 1);
	}

	void ReadFromMsg(const idBitMsg& msg) {
		localUserHandle = msg.ReadLong();
		lobbyType = static_cast<std::byte>(msg.ReadBytes(1));
	}
	
	//void Serialize(idSerializer& ser);

private:
	uint32_t localUserHandle = 0;
	std::byte lobbyType;
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

	virtual lobbyUserID_t GetLobbyUserIdByOrdinal(int userIndex) const = 0;

	// Lobby user access
	virtual bool IsLobbyUserValid(lobbyUserID_t lobbyUserID) const = 0;
	virtual bool IsLobbyUserLoaded(lobbyUserID_t lobbyUserID) const = 0;

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
	virtual void FindOrCreateMatch(const idMatchParameters& parms_) = 0;
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

	// GetActivePlatformLobbyBase will return either the game or party lobby, it won't return the game state lobby
	// This function is generally used for menus, in-game code should refer to GetActingGameStateLobby
	virtual idLobbyBase& GetActivePlatformLobbyBase() = 0;

	virtual sessionState_t GetState() const = 0;

	//virtual void SendUsercmds(idBitMsg& msg) = 0;
	virtual void SendSnapshot(class idSnapShot& ss) = 0;

	virtual void UpdateSignInManager() = 0;

	virtual void RegisterLocalUser() = 0;
	virtual bool IsLocalUserRegistered() = 0;
};

extern idSession* session;

#endif