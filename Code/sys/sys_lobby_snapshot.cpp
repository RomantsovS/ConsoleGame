#include "idlib/precompiled.h"
#include "sys_lobby.h"

/*
========================
idLobby::UpdateSnaps
========================
*/
void idLobby::UpdateSnaps() {
}

idCVar net_forceDropSnap("net_forceDropSnap", "0", CVAR_BOOL, "wait on snaps");

/*
========================
idLobby::SendSnapshotToPeer
========================
*/
void idLobby::SendSnapshotToPeer(idSnapShot& ss, int p) {
	assert(lobbyType == GetActingGameStateLobbyType());

	peer_t& peer = peers[p];

	if (net_forceDropSnap.GetBool()) {
		net_forceDropSnap.SetBool(false);
		return;
	}

	/*if (peer.pauseSnapshots) {
		return;
	}*/

	int time = Sys_Milliseconds();
}