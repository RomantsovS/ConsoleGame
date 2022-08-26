#include "idlib/precompiled.h"
#include "../framework/Common_local.h"
#include "sys_session_local.h"

/*
========================
idSessionLocalCallbacks::CreateLobbyBackend
========================
*/
idLobbyBackend* idSessionLocalCallbacks::CreateLobbyBackend(const idMatchParameters& p, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) {
	return sessionLocal->CreateLobbyBackend(p, skillLevel, lobbyType);
}

/*
========================
idSessionLocalCallbacks::DestroyLobbyBackend
========================
*/
void idSessionLocalCallbacks::DestroyLobbyBackend(idLobbyBackend* lobbyBackend) {
	sessionLocal->DestroyLobbyBackend(lobbyBackend);
}