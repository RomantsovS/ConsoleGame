#include "idlib/precompiled.h"

#include "Common_local.h"

idCVar net_snapRate("net_snapRate", "100", CVAR_SYSTEM | CVAR_INTEGER, "How many milliseconds between sending snapshots");

/*
===============
idCommonLocal::IsMultiplayer
===============
*/
bool idCommonLocal::IsMultiplayer() {
	idLobbyBase& lobby = session->GetPartyLobbyBase();
	return (((lobby.GetMatchParms().matchFlags &
		static_cast<int>(matchFlags_t::MATCH_ONLINE)) != 0) &&
		(session->GetState() > idSession::sessionState_t::IDLE));
}

/*
===============
idCommonLocal::IsServer
===============
*/
bool idCommonLocal::IsServer() {
	return IsMultiplayer() && session->GetActingGameStateLobbyBase().IsHost();
}

/*
===============
idCommonLocal::IsClient
===============
*/
bool idCommonLocal::IsClient() {
	return IsMultiplayer() && session->GetActingGameStateLobbyBase().IsPeer();
}

/*
===============
idCommonLocal::SendSnapshots
===============
*/
void idCommonLocal::SendSnapshots() {
	if (!mapSpawned) {
		return;
	}
	int currentTime = Sys_Milliseconds();
	if (currentTime < nextSnapshotSendTime) {
		return;
	}
	idLobbyBase& lobby = session->GetActingGameStateLobbyBase();
	if (!lobby.IsHost()) {
		return;
	}
	if (!lobby.HasActivePeers()) {
		return;
	}
	idSnapShot ss;
	game->ServerWriteSnapshot(ss);

	session->SendSnapshot(ss);
	nextSnapshotSendTime = MSEC_ALIGN_TO_FRAME(currentTime + net_snapRate.GetInteger());
}

/*
========================
idCommonLocal::ResetNetworkingState
========================
*/
void idCommonLocal::ResetNetworkingState() {
	userCmdMgr.SetDefaults();

	gameFrame = 0;
	//nextUsercmdSendTime = 0;
	nextSnapshotSendTime = 0;
}