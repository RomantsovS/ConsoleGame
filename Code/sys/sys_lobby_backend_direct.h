#ifndef	__SYS_LOBBY_BACKEND_DIRECT_H__
#define	__SYS_LOBBY_BACKEND_DIRECT_H__

/*
========================
idLobbyBackendDirect
========================
*/
class idLobbyBackendDirect : public idLobbyBackend {
public:
	idLobbyBackendDirect();

	// idLobbyBackend interface
	void StartHosting(const idMatchParameters& p, float skillLevel, lobbyBackendType_t type) override;
	void Shutdown() override;
	lobbyBackendState_t	GetState() override { return state; }
private:
	lobbyBackendState_t state;
	//netadr_t address;
};

#endif	// __SYS_LOBBY_BACKEND_DIRECT_H__