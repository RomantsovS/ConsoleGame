#include "idlib/precompiled.h"

#include "Common_local.h"

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
========================
idCommonLocal::ResetNetworkingState
========================
*/
void idCommonLocal::ResetNetworkingState() {
	userCmdMgr.SetDefaults();

	gameFrame = 0;
}