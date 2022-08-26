#include "idlib/precompiled.h"
#include "sys_lobby_backend.h"
#include "sys_lobby_backend_direct.h"

extern idCVar net_port;

/*
========================
idLobbyBackendDirect::idLobbyBackendDirect
========================
*/
idLobbyBackendDirect::idLobbyBackendDirect() {
	state = lobbyBackendState_t::STATE_INVALID;
}

/*
========================
idLobbyBackendDirect::StartHosting
========================
*/
void idLobbyBackendDirect::StartHosting(const idMatchParameters& p, float skillLevel, lobbyBackendType_t type) {
	NET_VERBOSE_PRINT("idLobbyBackendDirect::StartHosting\n");

	isLocal = true;// MatchTypeIsLocal(p.matchFlags);
	isHost = true;

	state = lobbyBackendState_t ::STATE_READY;
	isLocal = true;
}

/*
========================
idLobbyBackendDirect::Shutdown
========================
*/
void idLobbyBackendDirect::Shutdown() {
	state = lobbyBackendState_t::STATE_SHUTDOWN;
}