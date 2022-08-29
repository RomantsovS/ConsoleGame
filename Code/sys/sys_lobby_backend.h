#ifndef	__SYS_LOBBY_BACKEND_H__
#define	__SYS_LOBBY_BACKEND_H__

extern idCVar net_verboseResource;
#define NET_VERBOSERESOURCE_PRINT if ( net_verboseResource.GetBool() ) idLib::Printf

extern idCVar net_verbose;
#define NET_VERBOSE_PRINT if ( net_verbose.GetBool() ) idLib::Printf

class lobbyAddress_t {
public:
	
};

class idNetSessionPort {
public:
	bool InitPort(int portNumber, bool useBackend);
	//bool ReadRawPacket(lobbyAddress_t& from, void* data, int& size, int maxSize);
	//void SendRawPacket(const lobbyAddress_t& to, const void* data, int size);

	bool IsOpen();
	void Close();

private:
	idUDP UDP;
};

struct lobbyUser_t {
};

/*
================================================
idLobbyBackend
This class interfaces with the various back ends for the different platforms
================================================
*/
class idLobbyBackend {
public:
	enum class lobbyBackendState_t {
		STATE_INVALID = 0,
		STATE_READY = 1,
		STATE_CREATING = 2,		// In the process of creating the lobby as a host
		STATE_SEARCHING = 3,		// In the process of searching for a lobby to join
		STATE_OBTAINING_ADDRESS = 4,		// In the process of obtaining the address of the lobby owner
		STATE_ARBITRATING = 5,		// Arbitrating
		STATE_SHUTTING_DOWN = 6,		// In the process of shutting down
		STATE_SHUTDOWN = 7,		// Was a host or peer at one point, now ready to be deleted
		STATE_FAILED = 8,		// Failure occurred
		NUM_STATES
	};

	static const char* GetStateString(lobbyBackendState_t state_) {
		static const char* stateToString[static_cast<int>(lobbyBackendState_t::NUM_STATES)] = {
			"STATE_INVALID",
			"STATE_READY",
			"STATE_CREATING",
			"STATE_SEARCHING",
			"STATE_OBTAINING_ADDRESS",
			"STATE_ARBITRATING",
			"STATE_SHUTTING_DOWN",
			"STATE_SHUTDOWN",
			"STATE_FAILED"
		};

		return stateToString[static_cast<int>(state_)];
	}

	enum lobbyBackendType_t {
		TYPE_PARTY = 0,
		TYPE_GAME = 1,
		TYPE_GAME_STATE = 2,
		TYPE_INVALID = 0xff,
	};

	idLobbyBackend() : type(lobbyBackendType_t::TYPE_INVALID), isHost(false), isLocal(false) {}
	idLobbyBackend(lobbyBackendType_t lobbyType) : type(lobbyType), isHost(false), isLocal(false) {}

	virtual void StartHosting(const idMatchParameters& p, float skillLevel, lobbyBackendType_t type) = 0;
	virtual void Shutdown() = 0;
	virtual void SetInGame(bool value) {}

	virtual lobbyBackendState_t	GetState() = 0;

	void SetLobbyType(lobbyBackendType_t lobbyType) { type = lobbyType; }
	lobbyBackendType_t GetLobbyType() const { return type; }

protected:
	lobbyBackendType_t type;
	idMatchParameters parms;
	bool isLocal; // True if this lobby is restricted to local play only (won't need and can't connect to online lobbies)
	bool isHost; // True if we created this lobby
};

#endif