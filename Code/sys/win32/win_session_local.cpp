#include "idlib/precompiled.h"
#include "../../framework/Common_local.h"
#include "../sys_session_local.h"
#include "../sys_lobby_backend_direct.h"
#include "win_local.h"

extern idCVar net_port;

/*
========================
idSessionLocalWin::idSessionLocalWin
========================
*/
class idSessionLocalWin : public idSessionLocal {
friend class idLobbyToSessionCBLocal;
public:
	// idSessionLocal interface
	void Initialize() noexcept override;
	void Shutdown() noexcept override;

	idNetSessionPort& GetPort(bool dedicated = false) override;
	idLobbyBackend* CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) override;
	idLobbyBackend* FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) override;
	idLobbyBackend* JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) override;

	virtual void PumpLobbies();
private:
	void EnsurePort();

	idLobbyBackend* CreateLobbyInternal(idLobbyBackend::lobbyBackendType_t lobbyType);
	void DestroyLobbyBackend(idLobbyBackend* lobbyBackend) override;

	std::array<std::unique_ptr<idLobbyBackend>, 3> lobbyBackends;

	idNetSessionPort port;
	bool canJoinLocalHost = false;
};

idSessionLocalWin sessionLocalWin;
idSession* session = &sessionLocalWin;

/*
========================
idLobbyToSessionCBLocal
========================
*/
class idLobbyToSessionCBLocal : public idLobbyToSessionCB {
public:
	idLobbyToSessionCBLocal(idSessionLocalWin* sessionLocalWin_) : sessionLocalWin(sessionLocalWin_) { }

	bool CanJoinLocalHost() const override { sessionLocalWin->EnsurePort(); return sessionLocalWin->canJoinLocalHost; }
	idLobbyBackend* GetLobbyBackend(idLobbyBackend::lobbyBackendType_t type) const override { return sessionLocalWin->lobbyBackends[type].get(); }

private:
	idSessionLocalWin* sessionLocalWin;
};

idLobbyToSessionCBLocal lobbyToSessionCBLocal(&sessionLocalWin);
idLobbyToSessionCB* lobbyToSessionCB = &lobbyToSessionCBLocal;

/*
========================
idSessionLocalWin::Initialize
========================
*/
void idSessionLocalWin::Initialize() noexcept {
	idSessionLocal::Initialize();

	GetPartyLobby().Initialize(idLobby::lobbyType_t::TYPE_PARTY, sessionCallbacks.get());
	GetGameLobby().Initialize(idLobby::lobbyType_t::TYPE_GAME, sessionCallbacks.get());
	GetGameStateLobby().Initialize(idLobby::lobbyType_t::TYPE_GAME_STATE, sessionCallbacks.get());
}

/*
========================
idSessionLocalWin::Shutdown
========================
*/
void idSessionLocalWin::Shutdown() noexcept {
	NET_VERBOSE_PRINT("NET: Shutdown\n");
	idSessionLocal::Shutdown();

	MoveToMainMenu();

	// Wait until we fully shutdown
	while (localState != state_t::STATE_IDLE && localState != state_t::STATE_PRESS_START) {
		Pump();
	}
}

/*
========================
idSessionLocalWin::EnsurePort
========================
*/
void idSessionLocalWin::EnsurePort() {
	// Init the port using reqular windows sockets
	if (port.IsOpen()) {
		return;		// Already initialized
	}

	if (port.InitPort(net_port.GetInteger(), false)) {
		canJoinLocalHost = false;
	}
	else {
		// Assume this is another instantiation on the same machine, and just init using any available port
		port.InitPort(PORT_ANY, false);
		canJoinLocalHost = true;
	}
}

/*
========================
idSessionLocalWin::GetPort
========================
*/
idNetSessionPort& idSessionLocalWin::GetPort(bool dedicated) {
	EnsurePort();
	return port;
}

/*
========================
idSessionLocalWin::CreateLobbyBackend
========================
*/
idLobbyBackend* idSessionLocalWin::CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) {
	idLobbyBackend* lobbyBackend = CreateLobbyInternal(lobbyType);
	lobbyBackend->StartHosting(p, skillLevel, lobbyType);
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::FindLobbyBackend
========================
*/
idLobbyBackend* idSessionLocalWin::FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) {
	idLobbyBackend* lobbyBackend = CreateLobbyInternal(lobbyType);
	lobbyBackend->StartFinding(p, numPartyUsers, skillLevel);
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::JoinFromConnectInfo
========================
*/
idLobbyBackend* idSessionLocalWin::JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) {
	idLobbyBackend* lobbyBackend = CreateLobbyInternal(lobbyType);
	lobbyBackend->JoinFromConnectInfo(connectInfo);
	return lobbyBackend;
}

/*
========================
idSessionLocalWin::DestroyLobbyBackend
========================
*/
void idSessionLocalWin::DestroyLobbyBackend(idLobbyBackend* lobbyBackend) {
	assert(lobbyBackend);
	assert(lobbyBackends[lobbyBackend->GetLobbyType()].get() == lobbyBackend);

	lobbyBackend->Shutdown();
	lobbyBackends[lobbyBackend->GetLobbyType()] = nullptr;
}

/*
========================
idSessionLocalWin::PumpLobbies
========================
*/
void idSessionLocalWin::PumpLobbies() {
	assert(lobbyBackends[idLobbyBackend::TYPE_PARTY] == nullptr || lobbyBackends[idLobbyBackend::TYPE_PARTY]->GetLobbyType() == idLobbyBackend::TYPE_PARTY);
	assert(lobbyBackends[idLobbyBackend::TYPE_GAME] == nullptr || lobbyBackends[idLobbyBackend::TYPE_GAME]->GetLobbyType() == idLobbyBackend::TYPE_GAME);
	assert(lobbyBackends[idLobbyBackend::TYPE_GAME_STATE] == nullptr || lobbyBackends[idLobbyBackend::TYPE_GAME_STATE]->GetLobbyType() == idLobbyBackend::TYPE_GAME_STATE);

	// Pump lobbyBackends
	for (int i = 0; i < lobbyBackends.size(); i++) {
		if (lobbyBackends[i] != nullptr) {
			//lobbyBackends[i]->Pump();
		}
	}
}

/*
========================
idSessionLocalWin::CreateLobbyInternal
========================
*/
idLobbyBackend* idSessionLocalWin::CreateLobbyInternal(idLobbyBackend::lobbyBackendType_t lobbyType) {
	EnsurePort();
	std::unique_ptr<idLobbyBackend> lobbyBackend = std::make_unique<idLobbyBackendDirect>();

	lobbyBackend->SetLobbyType(lobbyType);

	assert(!lobbyBackends[lobbyType]);
	lobbyBackends[lobbyType] = std::move(lobbyBackend);

	return lobbyBackends[lobbyType].get();
}