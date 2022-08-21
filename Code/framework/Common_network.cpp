#include "idlib/precompiled.h"

#include "Common_local.h"

/*
===============
idCommonLocal::IsMultiplayer
===============
*/
bool idCommonLocal::IsMultiplayer() {
	/*idLobbyBase& lobby = session->GetPartyLobbyBase();
	return (((lobby.GetMatchParms().matchFlags & MATCH_ONLINE) != 0) && (session->GetState() > idSession::IDLE));*/
	return false;
}

/*
===============
idCommonLocal::IsServer
===============
*/
bool idCommonLocal::IsServer() {
	//return IsMultiplayer() && session->GetActingGameStateLobbyBase().IsHost();
	return false;
}

/*
===============
idCommonLocal::IsClient
===============
*/
bool idCommonLocal::IsClient() {
	//return IsMultiplayer() && session->GetActingGameStateLobbyBase().IsPeer();
	return false;
}