#include "idlib/precompiled.h"
#include "sys_lobby.h"

extern idCVar net_connectTimeoutInSeconds;
extern idCVar net_headlessServer;

/*
========================
idLobby::idLobby
========================
*/
idLobby::idLobby() {
	lobbyType = lobbyType_t::TYPE_INVALID;

	state = lobbyState_t::STATE_IDLE;
	failedReason = failedReason_t::FAILED_UNKNOWN;

	host = -1;
	peerIndexOnHost = -1;
	isHost = false;

	loaded = false;
}

/*
========================
idLobby::Initialize
========================
*/
void idLobby::Initialize(lobbyType_t sessionType_, idSessionCallbacks* callbacks) {
	assert(callbacks != nullptr);

	lobbyType = sessionType_;
	sessionCB = callbacks;

	if (lobbyType == GetActingGameStateLobbyType()) {
		// only needed in multiplayer mode
		//objMemory = (uint8*)Mem_Alloc(SNAP_OBJ_JOB_MEMORY, TAG_NETWORKING);
		//lzwData = (lzwCompressionData_t*)Mem_Alloc(sizeof(lzwCompressionData_t), TAG_NETWORKING);
	}
}

/*
========================
idLobby::StartHosting
========================
*/
void idLobby::StartHosting(const idMatchParameters& parms_) {
	parms = parms_;

	// Allow common to modify the parms
	common->OnStartHosting(parms);

	Shutdown();		// Make sure we're in a shutdown state before proceeding

	assert(GetNumLobbyUsers() == 0);
	assert(!lobbyBackend);

	// Get the skill level of all the players that will eventually go into the lobby
	StartCreating();
}

/*
========================
idLobby::Pump
========================
*/
void idLobby::Pump() {

	//UpdateLocalSessionUsers();

	switch (state) {
	case lobbyState_t::STATE_IDLE:					State_Idle();					break;
	case lobbyState_t::STATE_CREATE_LOBBY_BACKEND:	State_Create_Lobby_Backend();	break;
	/*case lobbyState_t::STATE_SEARCHING:				State_Searching();				break;
	case lobbyState_t::STATE_OBTAINING_ADDRESS:		State_Obtaining_Address();		break;
	case lobbyState_t::STATE_CONNECT_HELLO_WAIT:		State_Connect_Hello_Wait();		break;
	case lobbyState_t::STATE_FINALIZE_CONNECT:		State_Finalize_Connect();		break;
	case lobbyState_t::STATE_FAILED:													break;*/
	default:
		idLib::Error("idLobby::Pump:  Unknown state: %s", stateToString[static_cast<int>(state)].c_str());
	}
}

/*
========================
idLobby::Shutdown
========================
*/
void idLobby::Shutdown(bool retainMigrationInfo, bool skipGoodbye) {

	failedReason = failedReason_t::FAILED_UNKNOWN;

	if (!lobbyBackend) {
		NET_VERBOSE_PRINT("NET: ShutdownLobby (already shutdown) (%s)\n", GetLobbyName().c_str());

		// If we don't have this lobbyBackend type, we better be properly shutdown for this lobby
		assert(GetNumLobbyUsers() == 0);
		assert(host == -1);
		//assert(peerIndexOnHost == -1);
		assert(!isHost);
		assert(lobbyType != GetActingGameStateLobbyType() || !loaded);
		//assert(lobbyType != GetActingGameStateLobbyType() || !respondToArbitrate);
		//assert(snapDeltaAckQueue.Num() == 0);

		// Make sure we don't have old peers connected to this lobby
		/*for (int p = 0; p < peers.Num(); p++) {
			assert(peers[p].GetConnectionState() == CONNECTION_FREE);
		}*/

		state = lobbyState_t::STATE_IDLE;

		return;
	}

	NET_VERBOSE_PRINT("NET: ShutdownLobby (%s)\n", GetLobbyName().c_str());

	//for (int p = 0; p < peers.Num(); p++) {
	//	if (peers[p].GetConnectionState() != CONNECTION_FREE) {
	//		SetPeerConnectionState(p, CONNECTION_FREE, skipGoodbye);		// This will send goodbye's
	//	}
	//}

	//// Remove any users that weren't handled in ResetPeers
	//// (this will happen as a client, because we won't get the reliable msg from the server since we are severing the connection)
	//for (int i = 0; i < GetNumLobbyUsers(); i++) {
	//	lobbyUser_t* user = GetLobbyUser(i);
	//	UnregisterUser(user);
	//}

	//FreeAllUsers();

	host = -1;
	//peerIndexOnHost = -1;
	isHost = false;

	//partyToken = 0;		// Reset our party token so we recompute
	loaded = false;
	//respondToArbitrate = false;
	//waitForPartyOk = false;
	//startLoadingFromHost = false;

	//snapDeltaAckQueue.Clear();

	// Shutdown the lobbyBackend
	if (!retainMigrationInfo) {
		sessionCB->DestroyLobbyBackend(lobbyBackend);
		lobbyBackend = nullptr;
	}

	state = lobbyState_t::STATE_IDLE;
}

/*
========================
idLobby::HasActivePeers
========================
*/
bool idLobby::HasActivePeers() const {
	for (int p = 0; p < peers.size(); p++) {
		if (peers[p].GetConnectionState() != connectionState_t::CONNECTION_FREE) {
			return true;
		}
	}

	return false;
}

const std::string idLobby::stateToString[static_cast<int>(lobbyState_t::NUM_STATES)] = {
	ASSERT_ENUM_STRING(lobbyState_t::STATE_IDLE, 0),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_CREATE_LOBBY_BACKEND, 1),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_SEARCHING, 2),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_OBTAINING_ADDRESS, 3),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_CONNECT_HELLO_WAIT, 4),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_FINALIZE_CONNECT, 5),
	ASSERT_ENUM_STRING(lobbyState_t::STATE_FAILED, 6),
};

/*
========================
idLobby::State_Idle
========================
*/
void idLobby::State_Idle() {
	// If lobbyBackend is in a failed state, shutdown, go to a failed state ourself, and return
	if (lobbyBackend && lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_FAILED) {
		HandleConnectionAttemptFailed();
		return;
	}
}

/*
========================
idLobby::State_Create_Lobby_Backend
========================
*/
void idLobby::State_Create_Lobby_Backend() {
	if (!verify(lobbyBackend)) {
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	assert(lobbyBackend);

	if (lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_CREATING) {
		return;		// Busy but valid
	}

	if (lobbyBackend->GetState() != idLobbyBackend::lobbyBackendState_t::STATE_READY) {
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	// Success
	InitStateLobbyHost();

	// Set state to idle to signify to session we are done creating
	SetState(lobbyState_t::STATE_IDLE);
}

/*
========================
idLobby::SetState
========================
*/
void idLobby::SetState(lobbyState_t newState) {
	assert(static_cast<int>(newState) < static_cast<int>(lobbyState_t::NUM_STATES));
	assert(static_cast<int>(state) < static_cast<int>(lobbyState_t::NUM_STATES));

	if (state == newState) {
		NET_VERBOSE_PRINT("NET: idLobby::SetState: State SAME %s for session %s\n", stateToString[static_cast<int>(newState)], GetLobbyName().c_str());
		return;
	}

	// Set the current state
	NET_VERBOSE_PRINT("NET: idLobby::SetState: State changing from %s to %s for session %s\n", stateToString[static_cast<int>(state)].c_str(),
		stateToString[static_cast<int>(newState)].c_str(), GetLobbyName().c_str());

	state = newState;
}

/*
========================
idLobby::StartCreating
========================
*/
void idLobby::StartCreating() {
	assert(lobbyBackend == nullptr);
	assert(state == lobbyState_t::STATE_IDLE);

	lobbyBackend = sessionCB->CreateLobbyBackend(parms, 0, (idLobbyBackend::lobbyBackendType_t)lobbyType);

	SetState(lobbyState_t::STATE_CREATE_LOBBY_BACKEND);
}

/*
========================
idLobby::HandleGoodbyeFromPeer
========================
*/
void idLobby::HandleConnectionAttemptFailed() {
	Shutdown();
	failedReason = failedReason_t::FAILED_CONNECT_FAILED;
	SetState(lobbyState_t::STATE_FAILED);
}

/*
========================
idLobby::InitStateLobbyHost
========================
*/
void idLobby::InitStateLobbyHost() {
	assert(lobbyBackend);

	// We will be the host
	isHost = true;

	if (net_headlessServer.GetBool()) {
		return;		// Don't add any players to headless server
	}

	// Initialize the initial user list for this lobby
	//InitSessionUsersFromLocalUsers(MatchTypeIsOnline(parms.matchFlags));

	// Set the session's hostAddress to the local players' address.
	/*const int myUserIndex = GetLobbyUserIndexByLocalUserHandle(sessionCB->GetSignInManager().GetMasterLocalUserHandle());
	if (myUserIndex != -1) {
		hostAddress = GetLobbyUser(myUserIndex)->address;
	}*/

	// Since we are the host, we have to register our initial session users with the lobby
	// All additional users will join through AddUsersFromMsg, and RegisterUser is handled in there from here on out.
	// Peers will add users exclusively through AddUsersFromMsg.
	/*for (int i = 0; i < GetNumLobbyUsers(); i++) {
		lobbyUser_t* user = GetLobbyUser(i);
		RegisterUser(user);
		if (lobbyType == TYPE_PARTY) {
			user->partyToken = GetPartyTokenAsHost();
		}
	}*/
}

/*
========================
idLobby::PumpPings
Host: Periodically determine the round-trip time for a packet to all peers, and tell everyone
	what everyone else's ping to the host is so they can display it in the UI.
Client: Indicate to the player when the server hasn't updated the ping values in too long.
	This is usually going to preceed a connection timeout.
========================
*/
void idLobby::PumpPings() {
	//if (IsHost()) {
	//	// Calculate ping to all peers
	//	PingPeers();
	//	// Send the hosts calculated ping values to each peer to everyone has updated ping times
	//	SendPingValues();
	//	// Do bandwidth testing 
	//	ServerUpdateBandwidthTest();
	//	// Send Migration Data
	//	SendMigrationGameData();
	//}
	//else if (IsPeer()) {
	//	ClientUpdateBandwidthTest();

	//	if (lastPingValuesRecvTime + PING_INTERVAL_MS + 1000 < Sys_Milliseconds() && migrationInfo.state == MIGRATE_NONE) {
	//		for (int userIndex = 0; userIndex < GetNumLobbyUsers(); ++userIndex) {
	//			lobbyUser_t* user = GetLobbyUser(userIndex);
	//			if (!verify(user != NULL)) {
	//				continue;
	//			}
	//			user->pingMs = 999999;
	//		}
	//	}
	//}
}

/*
========================
idLobby::PumpPackets
========================
*/
void idLobby::PumpPackets() {
	int newTime = Sys_Milliseconds();

	//for (int p = 0; p < peers.Num(); p++) {
	//	if (peers[p].IsConnected()) {
	//		peers[p].packetProc->RefreshRates(newTime);
	//	}
	//}

	//// Resend reliable msg's (do this before we send out the fragments)
	//for (int p = 0; p < peers.Num(); p++) {
	//	ResendReliables(p);
	//}

	//// If we haven't sent anything to our peers in a long time, make sure to send an empty packet (so our heartbeat gets updated) so we don't get disconnected
	//// NOTE - We used to only send these to the host, but the host needs to also send these to clients
	//for (int p = 0; p < peers.Num(); p++) {
	//	if (!peers[p].IsConnected() || peers[p].packetProc->HasMoreFragments()) {
	//		continue;
	//	}
	//	if (newTime - peers[p].lastProcTime > 1000 * PEER_HEARTBEAT_IN_SECONDS) {
	//		//NET_VERBOSE_PRINT( "NET: ProcessOutgoing Heartbeat %s\n", GetLobbyName() );
	//		ProcessOutgoingMsg(p, NULL, 0, false, 0);
	//	}
	//}

	//// Send any unsent fragments for each peer (do this last)
	//for (int p = 0; p < peers.Num(); p++) {
	//	SendAnotherFragment(p);
	//}
}

/*
========================
idLobby::GetActingGameStateLobbyType
========================
*/
idLobby::lobbyType_t idLobby::GetActingGameStateLobbyType() const {
	extern idCVar net_useGameStateLobby;
	return (net_useGameStateLobby.GetBool()) ? lobbyType_t::TYPE_GAME_STATE : lobbyType_t::TYPE_GAME;
}

/*
========================
idLobby::peer_t::GetConnectionState
========================
*/
idLobby::connectionState_t idLobby::peer_t::GetConnectionState() const {
	return connectionState;
}