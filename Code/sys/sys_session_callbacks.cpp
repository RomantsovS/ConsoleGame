#include "idlib/precompiled.h"
#include "../framework/Common_local.h"
#include "sys_session_local.h"

/*
========================
idSessionLocalCallbacks::GoodbyeFromHost
========================
*/
void idSessionLocalCallbacks::GoodbyeFromHost(idLobby& lobby, int peerNum, const lobbyAddress_t& remoteAddress, int msgType) {
	sessionLocal->GoodbyeFromHost(lobby, peerNum, remoteAddress, msgType);
}

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
idSessionLocalCallbacks::FindLobbyBackend
========================
*/
idLobbyBackend* idSessionLocalCallbacks::FindLobbyBackend(const idMatchParameters& p, int numPartyUsers, float skillLevel, idLobbyBackend::lobbyBackendType_t lobbyType) {
	return sessionLocal->FindLobbyBackend(p, numPartyUsers, skillLevel, lobbyType);
}

/*
========================
idSessionLocalCallbacks::JoinFromConnectInfo
========================
*/
idLobbyBackend* idSessionLocalCallbacks::JoinFromConnectInfo(const lobbyConnectInfo_t& connectInfo, idLobbyBackend::lobbyBackendType_t lobbyType) {
	return sessionLocal->JoinFromConnectInfo(connectInfo, lobbyType);
}

/*
========================
idSessionLocalCallbacks::DestroyLobbyBackend
========================
*/
void idSessionLocalCallbacks::DestroyLobbyBackend(idLobbyBackend* lobbyBackend) {
	sessionLocal->DestroyLobbyBackend(lobbyBackend);
}