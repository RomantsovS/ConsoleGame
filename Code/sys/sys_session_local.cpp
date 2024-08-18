#include "idlib/precompiled.h"

#include "sys_session_local.h"
#include <ConnectionRequest.pb.h>
#include "sys_lobby_backend_direct.h"

idCVar net_useGameStateLobby("net_useGameStateLobby", "0", CVAR_BOOL, "");

idCVar net_port("net_port", "27015", CVAR_INTEGER, "host port number"); // Port to host when using dedicated servers, port to broadcast on when looking for a dedicated server to connect to
idCVar net_headlessServer("net_headlessServer", "0", CVAR_BOOL, "toggle to automatically host a game and allow peer[0] to control menus");

const std::string idSessionLocal::stateToString[static_cast<int>(state_t::NUM_STATES)] = {
	idassert_ENUM_STRING(state_t::STATE_PRESS_START, 0),
	idassert_ENUM_STRING(state_t::STATE_IDLE, 1),
	idassert_ENUM_STRING(state_t::STATE_PARTY_LOBBY_HOST, 2),
	idassert_ENUM_STRING(state_t::STATE_PARTY_LOBBY_PEER, 3),
	idassert_ENUM_STRING(state_t::STATE_GAME_LOBBY_HOST, 4),
	idassert_ENUM_STRING(state_t::STATE_GAME_LOBBY_PEER, 5),
	idassert_ENUM_STRING(state_t::STATE_GAME_STATE_LOBBY_HOST, 6),
	idassert_ENUM_STRING(state_t::STATE_GAME_STATE_LOBBY_PEER, 7),
	idassert_ENUM_STRING(state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY, 8),
	idassert_ENUM_STRING(state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY, 9),
	idassert_ENUM_STRING(state_t::STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY, 10),
	idassert_ENUM_STRING(state_t::STATE_FIND_OR_CREATE_MATCH, 11),
	idassert_ENUM_STRING(state_t::STATE_CONNECT_AND_MOVE_TO_PARTY, 12),
	idassert_ENUM_STRING(state_t::STATE_CONNECT_AND_MOVE_TO_GAME, 13),
	idassert_ENUM_STRING(state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE, 14),
	idassert_ENUM_STRING(state_t::STATE_BUSY, 15),
	idassert_ENUM_STRING(state_t::STATE_LOADING, 16),
	idassert_ENUM_STRING(state_t::STATE_INGAME, 17),
};

/*
========================
idSessionLocal::idSessionLocal
========================
*/
idSessionLocal::idSessionLocal() {
	InitBaseState();
}

/*
========================
idSessionLocal::InitBaseState
========================
*/
void idSessionLocal::InitBaseState() noexcept {
	localState = state_t::STATE_PRESS_START;

	sessionCallbacks = std::make_unique<idSessionLocalCallbacks>(this);

	//connectType = CONNECT_NONE;
	connectTime = 0;
}

/*
========================
idSessionLocal::FinishDisconnect
========================
*/
void idSessionLocal::FinishDisconnect() noexcept {
	GetPort().Close();
	/*while (sendQueue.Peek() != NULL) {
		sendQueue.RemoveFirst();
	}
	while (recvQueue.Peek() != NULL) {
		recvQueue.RemoveFirst();
	}*/
}

idCVar net_connectTimeoutInSeconds("net_connectTimeoutInSeconds", "15", CVAR_INTEGER, "timeout (in seconds) while connecting");

/*
========================
idSessionLocal::CreatePartyLobby
========================
*/
void idSessionLocal::CreatePartyLobby(const idMatchParameters& parms_) {
	NET_VERBOSE_PRINT("NET: CreatePartyLobby\n");

	// Shutdown any possible party lobby
	GetPartyLobby().Shutdown();

	// Shutdown any possible game lobby
	GetGameLobby().Shutdown();
	GetGameStateLobby().Shutdown();

	// Start hosting a new party lobby
	GetPartyLobby().StartHosting(parms_);

	//connectType = CONNECT_NONE;
	connectTime = Sys_Milliseconds();

	// Wait for it to complete
	SetState(state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY);
}

/*
========================
idSessionLocal::CreateMatch
========================
*/
void idSessionLocal::CreateMatch(const idMatchParameters& p) {
	NET_VERBOSE_PRINT("NET: CreateMatch\n");

	if ((p.matchFlags & static_cast<int>(matchFlags_t::MATCH_PARTY_INVITE_PLACEHOLDER)) && !GetPartyLobby().IsLobbyActive()) {
		NET_VERBOSE_PRINT("NET: CreateMatch MATCH_PARTY_INVITE_PLACEHOLDER\n");
		CreatePartyLobby(p);
		//connectType = CONNECT_NONE;
		return;
	}

	// Shutdown any possible game lobby
	GetGameLobby().Shutdown();
	GetGameStateLobby().Shutdown();

	// Start hosting a new game lobby
	GetGameLobby().StartHosting(p);

	//connectType = CONNECT_NONE;
	connectTime = Sys_Milliseconds();

	// Wait for it to complete
	SetState(state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY);
}

/*
========================
idSessionLocal::FindOrCreateMatch
========================
*/
void idSessionLocal::FindOrCreateMatch(const idMatchParameters& p) {
	NET_VERBOSE_PRINT("NET: FindOrCreateMatch\n");

	/*if ((p.matchFlags & MATCH_PARTY_INVITE_PLACEHOLDER) && !GetPartyLobby().IsLobbyActive()) {
		NET_VERBOSE_PRINT("NET: FindOrCreateMatch MATCH_PARTY_INVITE_PLACEHOLDER\n");
		CreatePartyLobby(p);
		connectType = CONNECT_FIND_OR_CREATE;
		return;
	}*/

	// Shutdown any possible game lobby
	GetGameLobby().Shutdown();
	GetGameStateLobby().Shutdown();

	// Start searching for a game
	GetGameLobby().StartFinding(p);

	//connectType = connectType_t::CONNECT_FIND_OR_CREATE;
	connectTime = Sys_Milliseconds();

	// Wait for searching to complete
	SetState(state_t::STATE_FIND_OR_CREATE_MATCH);
}

/*
========================
idSessionLocal::StartLoading
========================
*/
void idSessionLocal::StartLoading() noexcept {
	SetState(state_t::STATE_LOADING);
}

/*
========================
idSessionLocal::StartMatch
========================
*/
void idSessionLocal::StartMatch() noexcept {
	NET_VERBOSE_PRINT("NET: StartMatch\n");

	if (net_headlessServer.GetBool()) {
		StartLoading();		// This is so we can force start matches on headless servers to test performance using bots
		return;
	}

	// Start loading
	StartLoading();
}

/*
========================
idSessionLocal::GetBackState
========================
*/
idSessionLocal::sessionState_t idSessionLocal::GetBackState() {
	sessionState_t currentState = GetState();

	const bool isInGameLobby = currentState == sessionState_t::GAME_LOBBY;
	const bool isInPartyLobby = currentState == sessionState_t::PARTY_LOBBY;
	const bool isInGame = currentState == sessionState_t::INGAME || currentState == sessionState_t::LOADING; // Counting loading as ingame as far as what back state to go to

	if (isInGame) {
		return sessionState_t::GAME_LOBBY;		// If in the game, go back to game lobby
	}

	if (!isInPartyLobby && isInGameLobby) {
		return sessionState_t::PARTY_LOBBY;		// If in the game lobby, and we should have a party lobby, and we are the host, go back to party lobby
	}

	if (currentState != sessionState_t::IDLE) {
		return sessionState_t::IDLE;			// From here, go to idle if we aren't there yet
	}

	return sessionState_t::PRESS_START;			// Otherwise, go back to press start
}

/*
========================
idSessionLocal::Cancel
========================
*/
void idSessionLocal::Cancel() {
	NET_VERBOSE_PRINT("NET: Cancel\n");

	if (localState == state_t::STATE_PRESS_START) {
		return;		// We're as far back as we can go
	}

	// See what state we need to go to
	switch (GetBackState()) {
	case sessionState_t::GAME_LOBBY:
		EndMatch();		// End current match to go to game lobby
		break;

	case sessionState_t::PARTY_LOBBY:
		if (GetPartyLobby().IsHost()) {
			//if (sessionOptions & OPTION_LEAVE_WITH_PARTY) {
			//	// NOTE - This will send a message on the team lobby channel, 
			//	// so it won't be affected by the fact that we're shutting down the game lobby
			//	GetPartyLobby().NotifyPartyOfLeavingGameLobby();
			//}
			//else {
			//	// Host wants to be alone, disconnect all peers from the party
			//	GetPartyLobby().DisconnectAllPeers();
			//}

			// End the game lobby, and go back to party lobby as host
			GetGameLobby().Shutdown();
			GetGameStateLobby().Shutdown();
			SetState(state_t::STATE_PARTY_LOBBY_HOST);

			// Always remove this flag.  SendGoodbye uses this to determine if we should send a "leave with party"
			// and we don't want this flag hanging around, and causing false positives when it's called in the future.
			// Make them set this each time.
			//sessionOptions &= ~OPTION_LEAVE_WITH_PARTY;
		}
		else {
			// If we aren't the host of a party and we want to go back to one, we need to create a party now
			CreatePartyLobby(GetPartyLobby().parms);
		}
		break;

	case sessionState_t::IDLE:
		// Go back to main menu
		GetGameLobby().Shutdown();
		GetGameStateLobby().Shutdown();
		GetPartyLobby().Shutdown();
		SetState(state_t::STATE_IDLE);
		break;

	case sessionState_t::PRESS_START:
		// Go back to press start/main
		GetGameLobby().Shutdown();
		GetGameStateLobby().Shutdown();
		GetPartyLobby().Shutdown();
		SetState(state_t::STATE_PRESS_START);
		break;
	}

	// Validate the current lobby immediately
	ValidateLobbies();
}

/*
========================
idSessionLocal::MoveToPressStart
========================
*/
void idSessionLocal::MoveToPressStart() {
	if (localState != state_t::STATE_PRESS_START) {
		MoveToMainMenu();
		session->FinishDisconnect();
		SetState(state_t::STATE_PRESS_START);
	}
}

/*
========================
idSessionLocal::MoveToMainMenu
========================
*/
void idSessionLocal::MoveToMainMenu() noexcept {
	GetPartyLobby().Shutdown();
	GetGameLobby().Shutdown();
	GetGameStateLobby().Shutdown();
	SetState(state_t::STATE_IDLE);
}

/*
========================
idSessionLocal::WaitOnLobbyCreate

Called from State_Create_And_Move_To_Party_Lobby and State_Create_And_Move_To_Game_Lobby and State_Create_And_Move_To_Game_State_Lobby.
This function will create the lobby, then wait for it to either succeed or fail.
========================
*/
bool idSessionLocal::WaitOnLobbyCreate(idLobby& lobby) {

	idassert(localState == state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY || localState == state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY
		|| localState == state_t::STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY);
	//idassert(connectType == CONNECT_FIND_OR_CREATE || connectType == CONNECT_NONE);

	if (lobby.GetState() == idLobby::lobbyState_t::STATE_FAILED) {
		NET_VERBOSE_PRINT("NET: idSessionLocal::WaitOnLobbyCreate lobby.GetState() == idLobby::STATE_FAILED (%s)\n", lobby.GetLobbyName().c_str());
		// If we failed to create a lobby, assume connection to backend service was lost
		MoveToMainMenu();
		return false;
	}

	if (DetectDisconnectFromService(true)) {
		return false;
	}

	if (lobby.GetState() != idLobby::lobbyState_t::STATE_IDLE) {
		return false;		// Valid but busy
	}

	NET_VERBOSE_PRINT("NET: idSessionLocal::WaitOnLobbyCreate SUCCESS (%s)\n", lobby.GetLobbyName().c_str());

	return true;
}

/*
========================
idSessionLocal::DetectDisconnectFromService
Called from CreateMatch/CreatePartyLobby/FindOrCreateMatch state machines
========================
*/
bool idSessionLocal::DetectDisconnectFromService(bool cancelAndShowMsg) {
	const int DETECT_SERVICE_DISCONNECT_TIMEOUT_IN_SECONDS = 30;

	// If we are taking too long, cancel the connection
	if (DETECT_SERVICE_DISCONNECT_TIMEOUT_IN_SECONDS > 0) {
		if (Sys_Milliseconds() - connectTime > 1000 * DETECT_SERVICE_DISCONNECT_TIMEOUT_IN_SECONDS) {
			NET_VERBOSE_PRINT("NET: idSessionLocal::DetectDisconnectFromService timed out\n");
			if (cancelAndShowMsg) {
				MoveToMainMenu();
			}

			return true;
		}
	}

	return false;
}

/*
========================
idSessionLocal::HandleConnectionFailed
Called anytime a connection fails, and does the right thing.
========================
*/
void idSessionLocal::HandleConnectionFailed(idLobby& lobby, bool wasFull) {
	idassert(localState == state_t::STATE_CONNECT_AND_MOVE_TO_PARTY || localState == state_t::STATE_CONNECT_AND_MOVE_TO_GAME
		|| localState == state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE);
	//idassert(connectType == CONNECT_FIND_OR_CREATE || connectType == CONNECT_DIRECT);

	MoveToMainMenu();
}

/*
========================
idSessionLocal::HandleConnectAndMoveToLobby
Called from State_Connect_And_Move_To_Party/State_Connect_And_Move_To_Game
========================
*/
bool idSessionLocal::HandleConnectAndMoveToLobby(idLobby& lobby) {
	idassert(localState == state_t::STATE_CONNECT_AND_MOVE_TO_PARTY || localState == state_t::STATE_CONNECT_AND_MOVE_TO_GAME
		|| localState == state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE);
	//idassert(connectType == CONNECT_FIND_OR_CREATE || connectType == CONNECT_DIRECT);

	if (lobby.GetState() == idLobby::lobbyState_t::STATE_FAILED) {
		// If we get here, we were trying to connect to a lobby (from state State_Connect_And_Move_To_Party/State_Connect_And_Move_To_Game)
		HandleConnectionFailed(lobby, false);
		return true;
	}

	if (lobby.GetState() != idLobby::lobbyState_t::STATE_IDLE) {
		return HandlePackets();	// Valid but busy
	}

	//
	// Past this point, we've connected to the lobby
	//

	// If we are connecting to a game lobby, see if we need to keep waiting as either a host or peer while we're confirming all party members made it
	if (lobby.lobbyType == idLobby::lobbyType_t::TYPE_GAME) {
		if (GetPartyLobby().IsHost()) {

			const int timeoutMs = net_connectTimeoutInSeconds.GetInteger() * 1000;

			if (timeoutMs != 0 && Sys_Milliseconds() - lobby.helloStartTime > timeoutMs) {
				// Took too long, move to next result, or create a game instead
				HandleConnectionFailed(lobby, false);
				return true;
			}

			int numUsersIn = 0;

			for (int i = 0; i < GetPartyLobby().GetNumLobbyUsers(); i++) {
				bool foundUser = false;

				lobbyUser_t* partyUser = GetPartyLobby().GetLobbyUser(i);

				/*for (int j = 0; j < GetGameLobby().GetNumLobbyUsers(); j++) {
					lobbyUser_t* gameUser = GetGameLobby().GetLobbyUser(j);

					if (GetGameLobby().IsSessionUserLocal(gameUser) || gameUser->address.Compare(partyUser->address, true)) {
						numUsersIn++;
						foundUser = true;
						break;
					}
				}

				idassert(!GetPartyLobby().IsSessionUserIndexLocal(i) || foundUser);*/
			}

			if (numUsersIn != GetPartyLobby().GetNumLobbyUsers()) {
				return HandlePackets();		// All users not in, keep waiting until all user make it, or we time out
			}

			NET_VERBOSE_PRINT("NET: All party members made it into the game lobby.\n");

			// Let all the party members know everyone made it, and it's ok to stay at this server
			for (int i = 0; i < GetPartyLobby().peers.size(); i++) {
				if (GetPartyLobby().peers[i].IsConnected()) {
					//GetPartyLobby().QueueReliableMessage(i, idLobby::RELIABLE_PARTY_CONNECT_OK);
				}
			}
		}
		else {
			if (!idverify(lobby.host != -1)) {
				MoveToMainMenu();
				//connectType = CONNECT_NONE;
				return false;
			}
		}
	}

	// Success
	switch (lobby.lobbyType) {
	case idLobby::lobbyType_t::TYPE_PARTY:
		SetState(state_t::STATE_PARTY_LOBBY_PEER);
		break;
	case idLobby::lobbyType_t::TYPE_GAME:
		SetState(state_t::STATE_GAME_LOBBY_PEER);
		break;
	case idLobby::lobbyType_t::TYPE_GAME_STATE:
		// As a host of the game lobby, it's our duty to notify our members to also join this game state lobby
		GetGameLobby().SendMembersToLobby(GetGameStateLobby(), false);
		SetState(state_t::STATE_GAME_STATE_LOBBY_PEER);
		break;
	}

	//connectType = CONNECT_NONE;

	return false;
}

/*
========================
idSessionLocal::State_Party_Lobby_Host
========================
*/
bool idSessionLocal::State_Party_Lobby_Host() {
	return HandlePackets();
}

/*
========================
idSessionLocal::State_Game_Lobby_Host
========================
*/
bool idSessionLocal::State_Game_Lobby_Host() {
	return HandlePackets();
}

/*
========================
idSessionLocal::State_Game_Lobby_Peer
========================
*/
bool idSessionLocal::State_Game_Lobby_Peer() {
	if (GetActingGameStateLobby().startLoadingFromHost) {

	//	VerifySnapshotInitialState();

		// Set loading flag back to false
		GetActingGameStateLobby().startLoadingFromHost = false;

		// Set state to loading
		SetState(state_t::STATE_LOADING);

	//	loadingID++;

		return true;
	}

	return HandlePackets();
}

/*
========================
idSessionLocal::State_Create_And_Move_To_Party_Lobby
========================
*/
bool idSessionLocal::State_Create_And_Move_To_Party_Lobby() {
	if (WaitOnLobbyCreate(GetPartyLobby())) {

		//if (GetPartyLobby().parms.matchFlags & MATCH_PARTY_INVITE_PLACEHOLDER) {
		//	// If this party lobby was for a placeholder, continue on with either finding or creating a game lobby
		//	if (connectType == CONNECT_FIND_OR_CREATE) {
		//		FindOrCreateMatch(GetPartyLobby().parms);
		//		return true;
		//	}
		//	else if (connectType == CONNECT_NONE) {
		//		CreateMatch(GetPartyLobby().parms);
		//		return true;
		//	}
		//}

		// Success
		SetState(state_t::STATE_PARTY_LOBBY_HOST);

		return true;
	}

	return HandlePackets();		// Valid but busy
}

/*
========================
idSessionLocal::State_Create_And_Move_To_Game_Lobby
========================
*/
bool idSessionLocal::State_Create_And_Move_To_Game_Lobby() {

	if (WaitOnLobbyCreate(GetGameLobby())) {
		// Success
		SetState(state_t::STATE_GAME_LOBBY_HOST);

		return true;
	}

	return false;
}

/*
========================
idSessionLocal::State_Find_Or_Create_Match
========================
*/
bool idSessionLocal::State_Find_Or_Create_Match() {
	//idassert(connectType == CONNECT_FIND_OR_CREATE);

	if (GetGameLobby().GetState() == idLobby::lobbyState_t::STATE_FAILED) {
		// Failed to find any games.  Create one instead (we're assuming this always gets called from FindOrCreateMatch
		CreateMatch(GetGameLobby().parms);
		return true;
	}

	if (DetectDisconnectFromService(true)) {
		return false;
	}

	if (GetGameLobby().GetState() != idLobby::lobbyState_t::STATE_IDLE) {
		return HandlePackets();		// Valid but busy
	}

	// Done searching, connect to the first search result
	if (!GetGameLobby().ConnectToNextSearchResult()) {
		// Failed to find any games.  Create one instead (we're assuming this always gets called from FindOrCreateMatch
		CreateMatch(GetGameLobby().parms);
		return true;
	}

	SetState(state_t::STATE_CONNECT_AND_MOVE_TO_GAME);

	return true;
}

/*
========================
idSessionLocal::State_Connect_And_Move_To_Game
========================
*/
bool idSessionLocal::State_Connect_And_Move_To_Game() {
	return HandleConnectAndMoveToLobby(GetGameLobby());
}

/*
========================
idSessionLocal::State_InGame
========================
*/
bool idSessionLocal::State_InGame() {
	return HandlePackets();
}

/*
========================
idSessionLocal::State_Loading
========================
*/
bool idSessionLocal::State_Loading() {

	HandlePackets();

	if (!GetActingGameStateLobby().loaded) {
		return false;
	}

	if (GetActingGameStateLobby().IsHost()) {
		bool everyoneLoaded = true;
		for (int p = 0; p < GetActingGameStateLobby().peers.size(); p++) {
			idLobby::peer_t& peer = GetActingGameStateLobby().peers[p];

			if (!peer.IsConnected()) {
				continue;		// We don't care about peers that aren't connected as a game session
			}

			if (!peer.loaded) {
				everyoneLoaded = false;
				continue;		// Don't waste time sending resources to a peer who hasn't loaded the map yet
			}

			//if (GetActingGameStateLobby().SendResources(p)) {
			//	everyoneLoaded = false;

			//	// if client is taking a LONG time to load up - give them the boot: they're just holding up the lunch line. Useful for loose assets playtesting.
			//	int time = Sys_Milliseconds();
			//	int maxLoadTime = net_maxLoadResourcesTimeInSeconds.GetInteger();
			//	if (maxLoadTime > 0 && peer.startResourceLoadTime + SEC2MS(maxLoadTime) < time) {
			//		NET_VERBOSERESOURCE_PRINT("NET: dropping client %i - %s because they took too long to load resources.\n Check 'net_maxLoadResourcesTimeInSeconds' to adjust the time allowed.\n", p, GetPeerName(p));
			//		GetActingGameStateLobby().DisconnectPeerFromSession(p);
			//		continue;
			//	}
			//}
		}
		if (!everyoneLoaded) {
			return false;
		}
	}
	else {
		// not sure how we got there, but we won't be receiving anything that could get us out of this state anymore
		// possible step towards fixing the join stalling/disconnect problems
		if (GetActingGameStateLobby().peers.size() == 0) {
			NET_VERBOSE_PRINT("NET: no peers in idSessionLocal::State_Loading - giving up\n");
			MoveToMainMenu();
		}
		// need at least a peer with a real connection
		bool haveOneGoodPeer = false;
		for (int p = 0; p < GetActingGameStateLobby().peers.size(); p++) {
			if (GetActingGameStateLobby().peers[p].IsConnected()) {
				haveOneGoodPeer = true;
				break;
			}
		}
		if (!haveOneGoodPeer) {
			NET_VERBOSE_PRINT("NET: no good peers in idSessionLocal::State_Loading - giving up\n");
			MoveToMainMenu();
		}

		return false;
	}

	// if we got here then we're the host and everyone indicated map load finished
	NET_VERBOSE_PRINT("NET: (loading) Starting Game\n");
	SetState(state_t::STATE_INGAME);		// NOTE - Only the host is in-game at this point, all peers will start becoming in-game when they receive their first full snap
	return true;
}

idCVar net_verbose("net_verbose", "0", CVAR_BOOL, "Print a bunch of message about the network session");

/*
========================
idSessionLocal::Initialize
========================
*/
void idSessionLocal::Initialize() noexcept {
}

/*
========================
idSessionLocal::Shutdown
========================
*/
void idSessionLocal::Shutdown() noexcept {
}

/*
========================
idSessionLocal::EndMatch
EndMatch is meant for the host to cleanly end a match and return to the lobby page
========================
*/
void idSessionLocal::EndMatch(bool premature /*=false*/) {
	if (idverify(GetActingGameStateLobby().IsHost())) {
		// Host quits back to game lobby, and will notify peers internally to do the same
		EndMatchInternal(premature);
	}
}

/*
========================
idSessionLocal::QuitMatchToTitle
QuitMatchToTitle will forcefully quit the match and return to the title screen.
========================
*/
void idSessionLocal::QuitMatchToTitle() noexcept {
	MoveToMainMenu();
}

/*
========================
idSessionLocal::EndMatchInternal
========================
*/
void idSessionLocal::EndMatchInternal(bool premature/*=false*/) {
	idassert(GetGameStateLobby().IsLobbyActive() == net_useGameStateLobby.GetBool());

	for (int p = 0; p < GetActingGameStateLobby().peers.size(); p++) {
		// If we are the host, increment the session ID.  The client will use a rolling check to catch it
		if (GetActingGameStateLobby().IsHost()) {
			if (GetActingGameStateLobby().peers[p].IsConnected()) {
				/*if (GetActingGameStateLobby().peers[p].packetProc != NULL) {
					GetActingGameStateLobby().peers[p].packetProc->VerifyEmptyReliableQueue(idLobby::RELIABLE_GAME_DATA, idLobby::RELIABLE_DUMMY_MSG);
				}
				GetActingGameStateLobby().peers[p].sessionID = GetActingGameStateLobby().IncrementSessionID(GetActingGameStateLobby().peers[p].sessionID);*/
			}
		}
		//GetActingGameStateLobby().peers[p].ResetMatchData();
	}

	//GetActingGameStateLobby().snapDeltaAckQueue.Clear();

	GetActingGameStateLobby().loaded = false;

	//gameLobbyWasCoalesced = false;		// Reset this back to false.  We use this so the lobby code doesn't randomly choose a map when we coalesce
	//numFullSnapsReceived = 0;

	if (GetActingGameStateLobby().IsLobbyActive() /* && (GetActingGameStateLobby().GetMatchParms().matchFlags & MATCH_REQUIRE_PARTY_LOBBY)*/) {
		// All peers need to remove disconnected users to stay in sync
		//GetActingGameStateLobby().CompactDisconnectedUsers();

		// Go back to the game lobby
		if (GetActingGameStateLobby().IsHost()) {
			// We want the game state host to go back to STATE_GAME_STATE_LOBBY_HOST, so he can wait on all his game state peers to leave
			SetState(GetGameStateLobby().IsHost() ? state_t::STATE_GAME_STATE_LOBBY_HOST : state_t::STATE_GAME_LOBBY_HOST); // We want the dedicated host to go back to STATE_GAME_STATE_LOBBY_HOST
		}
		else {
			SetState(state_t::STATE_GAME_LOBBY_PEER);
		}
	}
	else {
		SetState(state_t::STATE_IDLE);
	}

	//if (GetActingGameStateLobby().IsHost()) {
	//	// Send a reliable msg to all peers to also "EndMatch"
	//	for (int p = 0; p < GetActingGameStateLobby().peers.Num(); p++) {
	//		GetActingGameStateLobby().QueueReliableMessage(p, premature ? idLobby::RELIABLE_ENDMATCH_PREMATURE : idLobby::RELIABLE_ENDMATCH);
	//	}
	//}
	//else if (premature) {
	//	// Notify client that host left early and thats why we are back in the lobby
	//	const bool stats = MatchTypeHasStats(GetActingGameStateLobby().GetMatchParms().matchFlags) && (GetFlushedStats() == false);
	//	common->Dialog().AddDialog(stats ? GDM_HOST_RETURNED_TO_LOBBY_STATS_DROPPED : GDM_HOST_RETURNED_TO_LOBBY, DIALOG_ACCEPT, NULL, NULL, false, __FUNCTION__, __LINE__, true);
	//}

	//if (GetGameStateLobby().IsLobbyActive()) {
	//	if (GetGameStateLobby().IsHost()) {
	//		// As a game state host, keep the lobby around, so we can make sure we know when everyone leaves (which means they got the reliable msg to EndMatch)
	//		waitingOnGameStateMembersToLeaveTime = Sys_Milliseconds();
	//	}
	//	else if (GetGameStateLobby().IsPeer()) {
	//		// Game state lobby peers should disconnect now
	//		GetGameStateLobby().Shutdown();
	//	}
	//}
}

/*
========================
idSessionLocal::ValidateLobbies
Determines if any of the session instances need to become the host
========================
*/
void idSessionLocal::ValidateLobbies() {
	if (localState == state_t::STATE_PRESS_START || localState == state_t::STATE_IDLE) {
		// At press start or main menu, don't do anything
		return;
	}

	if (GetActivePlatformLobby() == nullptr) {
		// If we're in between lobbies, don't do anything yet (the state transitioning code will handle error cases)
		return;
	}

	// Validate lobbies that should be alive and active
	if (/*ShouldHavePartyLobby() &&*/ GetState() >= idSession::sessionState_t::PARTY_LOBBY) {
		ValidateLobby(GetPartyLobby());
	}
	if (GetState() >= idSession::sessionState_t::GAME_LOBBY && !net_headlessServer.GetBool()) {
		ValidateLobby(GetGameLobby());
	}
}

/*
========================
idSessionLocal::ValidateLobby
========================
*/
void idSessionLocal::ValidateLobby(idLobby& lobby) {
	if (!lobby.lobbyBackend || lobby.lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_FAILED
		|| lobby.GetState() == idLobby::lobbyState_t::STATE_FAILED) {
		NET_VERBOSE_PRINT("NET: ValidateLobby: FAILED (lobbyType = %i, state = %s)\n", lobby.lobbyType, stateToString[static_cast<int>(localState)].c_str());
		if (lobby.failedReason == idLobby::failedReason_t::FAILED_MIGRATION_CONNECT_FAILED || lobby.failedReason == idLobby::failedReason_t::FAILED_CONNECT_FAILED) {
			MoveToMainMenu();
			//common->Dialog().AddDialog(GDM_INVALID_INVITE, DIALOG_ACCEPT, NULL, NULL, false);		// The game session no longer exists
		}
		else {
			// If the lobbyBackend goes bad under our feet for no known reason, assume we lost connection to the back end service
			MoveToMainMenu();
			//common->Dialog().ClearDialogs(true);
			//common->Dialog().AddDialog(GDM_CONNECTION_LOST, DIALOG_ACCEPT, NULL, NULL, false);		// Lost connection to XBox LIVE
		}
	}
}

/*
========================
idSessionLocal::Pump
========================
*/
void idSessionLocal::Pump() {
	static int lastPumpTime = -1;

	const int time = Sys_Milliseconds();
	const int elapsedPumpSeconds = (time - lastPumpTime) / 1000;

	if (lastPumpTime != -1 && elapsedPumpSeconds > 2) {
		idLib::Warning("idSessionLocal::Pump was not called for %i seconds", elapsedPumpSeconds);
	}

	lastPumpTime = time;

	bool shouldContinue = true;

	while (shouldContinue) {
		// Each iteration, validate the session instances
		ValidateLobbies();

		// Pump state
		shouldContinue = HandleState();

		// Pump lobbies
		PumpLobbies();
	}

	if (GetPartyLobby().lobbyBackend) {
		// Make sure game properties aren't set on the lobbyBackend if we aren't in a game lobby.
		// This is so we show up properly in search results in Play with Friends option
		GetPartyLobby().lobbyBackend->SetInGame(GetGameLobby().IsLobbyActive());

		// Temp location
		//UpdateMasterUserHeadsetState();
	}

	// Do some last minute checks, make sure everything about the current state and lobbyBackend state is valid, otherwise, take action
	ValidateLobbies();

	GetActingGameStateLobby().UpdateSnaps();

	idLobby* activeLobby = GetActivePlatformLobby();

	// Pump pings for the active lobby
	if (activeLobby != nullptr) {
		activeLobby->PumpPings();
	}

	// Pump packet processing for all lobbies
	GetPartyLobby().PumpPackets();
	GetGameLobby().PumpPackets();
	GetGameStateLobby().PumpPackets();

	int currentTime = Sys_Milliseconds();

	// Update possible pending invite
	//UpdatePendingInvite();

	// Check to see if we should coalesce the lobby
	//if (nextGameCoalesceTime != 0) {

	//	if (GetGameLobby().IsLobbyActive() &&
	//		GetGameLobby().IsHost() &&
	//		GetState() == idSession::GAME_LOBBY &&
	//		GetPartyLobby().GetNumLobbyUsers() <= 1 &&
	//		GetGameLobby().GetNumLobbyUsers() == 1 &&
	//		MatchTypeIsRanked(GetGameLobby().parms.matchFlags) &&
	//		Sys_Milliseconds() > nextGameCoalesceTime) {

	//		// If the player doesn't care about the mode or map,
	//		// make sure the search is broadened.
	//		idMatchParameters newGameParms = GetGameLobby().parms;
	//		newGameParms.gameMap = GAME_MAP_RANDOM;

	//		// Assume that if the party lobby's mode is random,
	//		// the player chose "Quick Match" and doesn't care about the mode.
	//		// If the player chose "Find Match" and a specific mode,
	//		// the party lobby mode will be set to non-random.
	//		if (GetPartyLobby().parms.gameMode == GAME_MODE_RANDOM) {
	//			newGameParms.gameMode = GAME_MODE_RANDOM;
	//		}

	//		FindOrCreateMatch(newGameParms);

	//		gameLobbyWasCoalesced = true;		// Remember that this round was coalesced.  We so this so main menu doesn't randomize the map, which looks odd
	//		nextGameCoalesceTime = 0;
	//	}
	//}
}

/*
========================
idSessionLocal::HandleState
========================
*/
bool idSessionLocal::HandleState() {
	// Handle individual lobby states
	GetPartyLobby().Pump();
	GetGameLobby().Pump();
	GetGameStateLobby().Pump();

	// Let IsHost be authoritative on the qualification of peer/host state types
	if (GetPartyLobby().IsHost() && localState == state_t::STATE_PARTY_LOBBY_PEER) {
		SetState(state_t::STATE_PARTY_LOBBY_HOST);
	}
	else if (GetPartyLobby().IsPeer() && localState == state_t::STATE_PARTY_LOBBY_HOST) {
		SetState(state_t::STATE_PARTY_LOBBY_PEER);
	}

	// Let IsHost be authoritative on the qualification of peer/host state types
	if (GetGameLobby().IsHost() && localState == state_t::STATE_GAME_LOBBY_PEER) {
		SetState(state_t::STATE_GAME_LOBBY_HOST);
	}
	else if (GetGameLobby().IsPeer() && localState == state_t::STATE_GAME_LOBBY_HOST) {
		SetState(state_t::STATE_GAME_LOBBY_PEER);
	}

	switch (localState) {
	case state_t::STATE_PRESS_START:							return false;
	case state_t::STATE_IDLE:								HandlePackets(); return false;		// Call handle packets, since packets from old sessions could still be in flight, which need to be emptied
	case state_t::STATE_PARTY_LOBBY_HOST:					return State_Party_Lobby_Host();
		//case state_t::STATE_PARTY_LOBBY_PEER:					return State_Party_Lobby_Peer();
	case state_t::STATE_GAME_LOBBY_HOST:						return State_Game_Lobby_Host();
	case state_t::STATE_GAME_LOBBY_PEER:						return State_Game_Lobby_Peer();
		/*case state_t::STATE_GAME_STATE_LOBBY_HOST:				return State_Game_State_Lobby_Host();
		case state_t::STATE_GAME_STATE_LOBBY_PEER:				return State_Game_State_Lobby_Peer();*/
	case state_t::STATE_LOADING:								return State_Loading();
	case state_t::STATE_INGAME:								return State_InGame();
	case state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY:		return State_Create_And_Move_To_Party_Lobby();
	case state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY:		return State_Create_And_Move_To_Game_Lobby();
		//case state_t::STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY:	return State_Create_And_Move_To_Game_State_Lobby();
		case state_t::STATE_FIND_OR_CREATE_MATCH:				return State_Find_Or_Create_Match();
		//case state_t::STATE_CONNECT_AND_MOVE_TO_PARTY:			return State_Connect_And_Move_To_Party();
		case state_t::STATE_CONNECT_AND_MOVE_TO_GAME:			return State_Connect_And_Move_To_Game();
		/*case state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE:		return State_Connect_And_Move_To_Game_State();
		case state_t::STATE_BUSY:								return State_Busy();*/
	default:
		idLib::Error("HandleState:  Unknown state in idSessionLocal: %s", stateToString[static_cast<int>(localState)].c_str());
	}
	return false;
}

/*
========================
idSessionLocal::GetState
========================
*/
idSessionLocal::sessionState_t idSessionLocal::GetState() const {
	// Convert our internal state to one of the external states
	switch (localState) {
	case state_t::STATE_PRESS_START:						return sessionState_t::PRESS_START;
	case state_t::STATE_IDLE:								return sessionState_t::IDLE;
	case state_t::STATE_PARTY_LOBBY_HOST:					return sessionState_t::PARTY_LOBBY;
	case state_t::STATE_PARTY_LOBBY_PEER:					return sessionState_t::PARTY_LOBBY;
	case state_t::STATE_GAME_LOBBY_HOST:					return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_LOBBY_PEER:					return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_STATE_LOBBY_HOST:				return sessionState_t::GAME_LOBBY;
	case state_t::STATE_GAME_STATE_LOBBY_PEER:				return sessionState_t::GAME_LOBBY;
	case state_t::STATE_LOADING:							return sessionState_t::LOADING;
	case state_t::STATE_INGAME:								return sessionState_t::INGAME;
	case state_t::STATE_CREATE_AND_MOVE_TO_PARTY_LOBBY:		return sessionState_t::CONNECTING;
	case state_t::STATE_CREATE_AND_MOVE_TO_GAME_LOBBY:		return sessionState_t::CONNECTING;
	case state_t::STATE_CREATE_AND_MOVE_TO_GAME_STATE_LOBBY:return sessionState_t::CONNECTING;
	case state_t::STATE_FIND_OR_CREATE_MATCH:				return sessionState_t::SEARCHING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_PARTY:			return sessionState_t::CONNECTING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_GAME:			return sessionState_t::CONNECTING;
	case state_t::STATE_CONNECT_AND_MOVE_TO_GAME_STATE:		return sessionState_t::CONNECTING;
	case state_t::STATE_BUSY:								return sessionState_t::BUSY;
	default: {
		common->Error("GetState: Unknown state in idSessionLocal");
	}
	};

	return sessionState_t::MAX_STATES;
}

/*
========================
idSessionLocal::SendSnapshot
========================
*/
void idSessionLocal::SendSnapshot(idSnapShot& ss) {
	for (int p = 0; p < GetActingGameStateLobby().peers.size(); p++) {
		idLobby::peer_t& peer = GetActingGameStateLobby().peers[p];

		if (!peer.IsConnected()) {
			continue;
		}

		if (!peer.loaded) {
			continue;
		}

		/*if (peer.pauseSnapshots) {
			continue;
		}*/

		GetActingGameStateLobby().SendSnapshotToPeer(ss, p);
	}
}

/*
========================
idSessionLocal::UpdateSignInManager
========================
*/
void idSessionLocal::UpdateSignInManager() {
	if (!localUserRegistered) {
		// If we don't have a master user at all, then we need to be at "Press Start"
		MoveToPressStart();
		return;
	}
	else if (localState == state_t::STATE_PRESS_START) {
		// If we have a master user, and we are at press start, move to the menu area
		SetState(state_t::STATE_IDLE);
	}
}

// idSession interface

/*
========================
idSessionLocal::LoadingFinished

Only called by idCommonLocal::FinalizeMapChange
========================
*/
void idSessionLocal::LoadingFinished() noexcept {
	NET_VERBOSE_PRINT("NET: Loading Finished\n");

	idassert(GetState() == idSession::sessionState_t::LOADING);

	GetActingGameStateLobby().loaded = true;

	if (!GetActingGameStateLobby().IsHost()) {	// Tell game host we're done loading
		std::array<std::byte, idPacketProcessor::MAX_PACKET_SIZE> buffer;
		idBitMsg msg(buffer.data(), sizeof(buffer));
		GetActingGameStateLobby().QueueReliableMessage(GetActingGameStateLobby().host, idLobby::reliableType_t::RELIABLE_LOADING_DONE, msg.GetReadData(), msg.GetSize());
	}
	else {
		SetState(state_t::STATE_INGAME);
	}
}

/*
========================
idSessionLocal::SetState
========================
*/
void idSessionLocal::SetState(state_t newState) noexcept {
	//idassert(newState < NUM_STATES);
	//idassert(localState < NUM_STATES);

	idLib::Printf("idSessionLocal::SetState: State changing from %s to %s\n", stateToString[static_cast<int>(localState)].c_str(),
		stateToString[static_cast<int>(newState)].c_str());

	localState = newState;
}

/*
========================
idSessionLocal::HandlePackets
========================
*/
bool idSessionLocal::HandlePackets() {
	std::array<std::byte, idPacketProcessor::MAX_FINAL_PACKET_SIZE> packetBuffer;
	lobbyAddress_t remoteAddress;
	int recvSize = 0;

	while (ReadRawPacket(remoteAddress, packetBuffer.data(), recvSize, packetBuffer.size()) && recvSize > 0) {
		idBitMsg fragMsg;
		fragMsg.InitRead(packetBuffer.data(), recvSize);

		// Peek at the session ID
		idPacketProcessor::sessionId_t sessionID = idPacketProcessor::GetSessionID(fragMsg);

		// Make sure it's valid
		if (sessionID == idPacketProcessor::SESSION_ID_INVALID) {
			idLib::Printf("NET: Invalid sessionID %s.\n", remoteAddress.ToString().c_str());
			continue;
		}

		//
		// Distribute the packet to the proper lobby
		//

		const int maskedType = sessionID & idPacketProcessor::LOBBY_TYPE_MASK;

		if (!idverify(maskedType > 0)) {
			continue;
		}

		idLobby::lobbyType_t lobbyType = static_cast<idLobby::lobbyType_t>(maskedType - 1);

		switch (lobbyType) {
			case idLobby::lobbyType_t::TYPE_GAME:
				GetGameLobby().HandlePacket(remoteAddress, fragMsg, sessionID);
				break;
			default:
				idassert(0);
		}
	}

	return false;
}

/*
========================
idSessionLocal::GetActivePlatformLobby
========================
*/
idLobby* idSessionLocal::GetActivePlatformLobby() {
	sessionState_t state = GetState();

	if ((state == sessionState_t::GAME_LOBBY) || (state == sessionState_t::BUSY) || (state == sessionState_t::INGAME) || (state == sessionState_t::LOADING)) {
		return &GetGameLobby();
	}
	else if (state == sessionState_t::PARTY_LOBBY) {
		return &GetPartyLobby();
	}

	return nullptr;
}

/*
========================
idSessionLocal::GetActingGameStateLobby
========================
*/
idLobby& idSessionLocal::GetActingGameStateLobby() {
	if (net_useGameStateLobby.GetBool()) {
		return GetGameStateLobby();
	}

	return GetGameLobby();
}

/*
========================
idSessionLocal::GetActivePlatformLobbyBase
This returns the base version for the idSession version
========================
*/
idLobbyBase& idSessionLocal::GetActivePlatformLobbyBase() {
	idLobby* activeLobby = GetActivePlatformLobby();

	if (activeLobby != nullptr) {
		return *activeLobby;
	}

	return stubLobby;		// So we can return at least something
}

/*
========================
idSessionLocal::SendRawPacket
========================
*/
void idSessionLocal::SendRawPacket(const lobbyAddress_t& to, const void* data, int size, bool dedicated) {
	const int now = Sys_Milliseconds();

	// short path
	// NOTE: network queuing: will go to tick the queue whenever sendQueue isn't empty, regardless of latency
	GetPort(dedicated).SendRawPacket(to, data, size);
}

/*
========================
idSessionLocal::ReadRawPacket
========================
*/
bool idSessionLocal::ReadRawPacket(lobbyAddress_t& from, void* data, int& size, int maxSize) {
	for (int i = 0; i < 2; i++) {
		if (GetPort(false).ReadRawPacket(from, data, size, maxSize)) {
			return true;
		}
	}

	return false;
}

/*
========================
idSessionLocal::GoodbyeFromHost
========================
*/
void idSessionLocal::GoodbyeFromHost(idLobby& lobby, int peerNum, const lobbyAddress_t& remoteAddress, int msgType) {
	if (!idverify(static_cast<int>(localState) > static_cast<int>(state_t::STATE_IDLE))) {
		idLib::Printf("NET: Got disconnected from host %s on session %s when we were not in a lobby or game.\n", remoteAddress.ToString().c_str(),
			lobby.GetLobbyName().c_str());
		MoveToMainMenu();
		return;		// Ignore if we are not past the main menu
	}

	// Goodbye from host.  See if we were connecting vs connected
	if ((localState == state_t::STATE_CONNECT_AND_MOVE_TO_PARTY || localState == state_t::STATE_CONNECT_AND_MOVE_TO_GAME)
		&& lobby.peers[peerNum].GetConnectionState() == idLobby::connectionState_t::CONNECTION_CONNECTING) {
		// We were denied a connection attempt
		idLib::Printf("NET: Denied connection attempt from host %s on session %s. MsgType %i.\n", remoteAddress.ToString().c_str(), lobby.GetLobbyName().c_str(), msgType);
		// This will try to move to the next connection if one exists, otherwise will create a match
		HandleConnectionFailed(lobby, msgType == idLobby::OOB_GOODBYE_FULL);
	}
	else {
		// We were disconnected from a server we were previously connected to
		idLib::Printf("NET: Disconnected from host %s on session %s. MsgType %i.\n", remoteAddress.ToString().c_str(), lobby.GetLobbyName().c_str(), msgType);

		// Host left, so pick a new host (possibly even us) for this lobby
		//lobby.PickNewHost();
	}
}

extern idCVar net_port;

idSessionLocalWin sessionLocalWin;
idSession* session = &sessionLocalWin;

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
	idassert(lobbyBackend);
	idassert(lobbyBackends[lobbyBackend->GetLobbyType()].get() == lobbyBackend);

	lobbyBackend->Shutdown();
	lobbyBackends[lobbyBackend->GetLobbyType()] = nullptr;
}

/*
========================
idSessionLocalWin::PumpLobbies
========================
*/
void idSessionLocalWin::PumpLobbies() {
	idassert(lobbyBackends[idLobbyBackend::TYPE_PARTY] == nullptr || lobbyBackends[idLobbyBackend::TYPE_PARTY]->GetLobbyType() == idLobbyBackend::TYPE_PARTY);
	idassert(lobbyBackends[idLobbyBackend::TYPE_GAME] == nullptr || lobbyBackends[idLobbyBackend::TYPE_GAME]->GetLobbyType() == idLobbyBackend::TYPE_GAME);
	idassert(lobbyBackends[idLobbyBackend::TYPE_GAME_STATE] == nullptr || lobbyBackends[idLobbyBackend::TYPE_GAME_STATE]->GetLobbyType() == idLobbyBackend::TYPE_GAME_STATE);

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

	idassert(!lobbyBackends[lobbyType]);
	lobbyBackends[lobbyType] = std::move(lobbyBackend);

	return lobbyBackends[lobbyType].get();
}

/*
========================
lobbyAddress_t::lobbyAddress_t
========================
*/
lobbyAddress_t::lobbyAddress_t() {
	memset(&netAddr, 0, sizeof(netAddr));
	netAddr.type = netadrtype_t::NA_BAD;
}

/*
========================
lobbyAddress_t::ToString
========================
*/
std::string lobbyAddress_t::ToString() const {
	std::ostringstream oss;
	oss << netAddr.address << ":" << netAddr.port;
	return oss.str();
}

/*
========================
lobbyAddress_t::Compare
========================
*/
bool lobbyAddress_t::Compare(const lobbyAddress_t& addr, bool ignoreSessionCheck) const {
	return Sys_CompareNetAdrBase(netAddr, addr.netAddr);
}

/*
========================
idNetSessionPort::InitPort
========================
*/
bool idNetSessionPort::InitPort(int portNumber, bool useBackend) {
	return UDP.InitForPort(portNumber);
}

/*
========================
idNetSessionPort::ReadRawPacket
========================
*/
bool idNetSessionPort::ReadRawPacket(lobbyAddress_t& from, void* data, int& size, int maxSize) {
	bool result = UDP.GetPacket(from.netAddr, data, size, maxSize);

	return result;
}

/*
========================
idNetSessionPort::SendRawPacket
========================
*/
void idNetSessionPort::SendRawPacket(const lobbyAddress_t to, const void* data, int size) {
	assert(size <= idPacketProcessor::MAX_FINAL_PACKET_SIZE);

	UDP.SendPacket(to.netAddr, data, size);
}

/*
========================
idNetSessionPort::IsOpen
========================
*/
bool idNetSessionPort::IsOpen() {
	return UDP.IsOpen();
}

/*
========================
idNetSessionPort::Close
========================
*/
void idNetSessionPort::Close() {
	UDP.Close();
}