#include "idlib/precompiled.h"
#include "sys_lobby.h"
#include <ConnectionRequest.pb.h>
#include <ReliableHello.pb.h>

extern idCVar net_connectTimeoutInSeconds;
extern idCVar net_headlessServer;

/*
========================
idLobby::idLobby
========================
*/
idLobby::idLobby() {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	lobbyType = lobbyType_t::TYPE_INVALID;

	state = lobbyState_t::STATE_IDLE;
	failedReason = failedReason_t::FAILED_UNKNOWN;

	host = -1;
	peerIndexOnHost = -1;
	isHost = false;

	loaded = false;
	startLoadingFromHost = false;

	peers.resize(MAX_PEERS);
	userPool.resize(MAX_PLAYERS);

	// Initialize free user list
	for (int i = 0; i < userPool.size(); i++) {
		freeUsers.push_back(&userPool[i]);
	}
}

/*
========================
idLobby::Initialize
========================
*/
void idLobby::Initialize(lobbyType_t sessionType_, idSessionCallbacks* callbacks) {
	idassert(callbacks != nullptr);

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

	idassert(GetNumLobbyUsers() == 0);
	idassert(!lobbyBackend);

	// Get the skill level of all the players that will eventually go into the lobby
	StartCreating();
}

/*
========================
idLobby::StartFinding
========================
*/
void idLobby::StartFinding(const idMatchParameters& parms_) {
	parms = parms_;

	Shutdown();		// Make sure we're in a shutdown state before proceeding

	idassert(GetNumLobbyUsers() == 0);
	idassert(!lobbyBackend);

	// Clear search results
	searchResults.clear();

	lobbyBackend = sessionCB->FindLobbyBackend(parms, sessionCB->GetPartyLobby().GetNumLobbyUsers(), 0,
		idLobbyBackend::lobbyBackendType_t::TYPE_GAME);

	SetState(lobbyState_t::STATE_SEARCHING);
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
	case lobbyState_t::STATE_SEARCHING:				State_Searching();				break;
	case lobbyState_t::STATE_OBTAINING_ADDRESS:		State_Obtaining_Address();		break;
	case lobbyState_t::STATE_CONNECT_HELLO_WAIT:		State_Connect_Hello_Wait();		break;
	case lobbyState_t::STATE_FINALIZE_CONNECT:		State_Finalize_Connect();		break;
	case lobbyState_t::STATE_FAILED:													break;
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
		idassert(GetNumLobbyUsers() == 0);
		idassert(host == -1);
		//idassert(peerIndexOnHost == -1);
		idassert(!isHost);
		idassert(lobbyType != GetActingGameStateLobbyType() || !loaded);
		//idassert(lobbyType != GetActingGameStateLobbyType() || !respondToArbitrate);
		//idassert(snapDeltaAckQueue.Num() == 0);

		// Make sure we don't have old peers connected to this lobby
		/*for (int p = 0; p < peers.Num(); p++) {
			idassert(peers[p].GetConnectionState() == CONNECTION_FREE);
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
	startLoadingFromHost = false;

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
idLobby::HandlePacket
========================
*/
void idLobby::HandlePacket(lobbyAddress_t& remoteAddress, idBitMsg& fragMsg, idPacketProcessor::sessionId_t sessionID) {
	// msg will hold a fully constructed msg using the packet processor
	std::array<std::byte, idPacketProcessor::MAX_MSG_SIZE> msgBuffer;

	idBitMsg msg;
	msg.InitWrite(msgBuffer.data(), sizeof(msgBuffer));

	int peerNum = FindPeer(remoteAddress, sessionID);
	int type = idPacketProcessor::RETURN_TYPE_NONE;
	int	userData = 0;

	if (peerNum >= 0) {
		if (!peers[peerNum].IsActive()) {
			idLib::Printf("NET: Received in-band packet from peer %s with no active connection.\n", remoteAddress.ToString());
			return;
		}
		type = peers[peerNum].packetProc->ProcessIncoming(Sys_Milliseconds(), peers[peerNum].sessionID, fragMsg, msg, userData, peerNum);
	}
	else {
		if (!idPacketProcessor::ProcessConnectionlessIncoming(fragMsg, msg, userData)) {
			idLib::Printf("ProcessConnectionlessIncoming FAILED from %s.\n", remoteAddress.ToString().c_str());
			// Not a valid connectionless packet
			return;
		}

		// Valid connectionless packets are always RETURN_TYPE_OOB
		type = idPacketProcessor::RETURN_TYPE_OOB;

		// Find the peer this connectionless msg should go to
		peerNum = FindPeer(remoteAddress, sessionID, true);
	}

	if (type == idPacketProcessor::RETURN_TYPE_NONE) {
		// This packet is not necessarily invalid, it could be a start or middle of a fragmented packet that's not fully constructed.
		return;
	}

	// Handle server query requests.  We do this before the STATE_IDLE check.  This is so we respond.
	// We may want to change this to just ignore the request if we are idle, and change the timeout time
	// on the requesters part to just timeout faster.
	/*if (type == idPacketProcessor::RETURN_TYPE_OOB) {
		if (userData == OOB_MATCH_QUERY || userData == OOB_SYSTEMLINK_QUERY) {
			sessionCB->HandleServerQueryRequest(remoteAddress, msg, userData);
			return;
		}
		if (userData == OOB_MATCH_QUERY_ACK) {
			sessionCB->HandleServerQueryAck(remoteAddress, msg);
			return;
		}
	}*/

	if (type == idPacketProcessor::RETURN_TYPE_OOB) {
		if (userData == OOB_HELLO) {
			// Handle new peer connect request
			peerNum = HandleInitialPeerConnection(msg, remoteAddress, peerNum);
			return;
		}
		else if (userData == OOB_GOODBYE || userData == OOB_GOODBYE_W_PARTY || userData == OOB_GOODBYE_FULL) {
			HandleGoodbyeFromPeer(peerNum, remoteAddress, userData);
			return;
		}
	}
	else if (type == idPacketProcessor::RETURN_TYPE_INBAND) {
		// Process in-band message
		if (peerNum < 0) {
			idLib::Printf("NET: In-band message from unknown peer: %s\n", remoteAddress.ToString().c_str());
			return;
		}

		if (!idverify(peers[peerNum].address.Compare(remoteAddress))) {
			idLib::Printf("NET: Peer with wrong address: %i, %s\n", peerNum, remoteAddress.ToString().c_str());
			return;
		}

		// Handle reliable
		int numReliable = peers[peerNum].packetProc->GetNumReliables();
		for (int r = 0; r < numReliable; r++) {
			// Just in case one of the reliable msg's cause this peer to disconnect
			// (this can happen when our party/game host is the same, he quits the game lobby, and sends a reliable msg for us to leave the game)
			peerNum = FindPeer(remoteAddress, sessionID);

			if (peerNum == -1) {
				idLib::Printf("NET: Dropped peer while processing reliable msg's: %i, %s\n", peerNum, remoteAddress.ToString().c_str());
				break;
			}

			const std::byte* reliableData = peers[peerNum].packetProc->GetReliable(r);
			int reliableSize = peers[peerNum].packetProc->GetReliableSize(r);
			idBitMsg reliableMsg(reliableData, reliableSize);
			reliableMsg.SetSize(reliableSize);

			HandleReliableMsg(peerNum, reliableMsg);
		}

		if (peerNum == -1 || !peers[peerNum].IsConnected()) {
			// If the peer still has no connection after HandleReliableMsg, then something is wrong.
			// (We could have been in CONNECTION_CONNECTING state for this session type, but the first message
			// we should receive from the server is the ack, otherwise, something went wrong somewhere)
			idLib::Printf("NET: In-band message from host with no active connection: %i, %s\n", peerNum, remoteAddress.ToString().c_str());
			return;
		}

		// Handle unreliable part (if any)
		if (msg.GetRemainingData() > 0 && loaded) {
			if (!idverify(lobbyType == GetActingGameStateLobbyType())) {
				idLib::Printf("NET: Snapshot msg for non game session lobby %s\n", remoteAddress.ToString().c_str());
				return;
			}

			if (peerNum == host) {
				idSnapShot localSnap;
				int sequence = -1;
				int baseseq = -1;
				bool fullSnap = false;
				//localReadSS = &localSnap;

				// If we are the peer, we assume we only receive snapshot data on the in-band channel
				const std::byte* deltaData = msg.GetReadData() + msg.GetReadCount();
				int deltaLength = msg.GetRemainingData();

				if (peers[peerNum].snapProc->ReceiveSnapshotDelta(deltaData, deltaLength, sequence, baseseq, localSnap, fullSnap)) {

					NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("NET: Got %s snapshot %d delta'd against %d. SS Time: %d\n", (!fullSnap ? "partial" : "full"),
						sequence, baseseq, localSnap.GetTime()).c_str());

					//if (sessionCB->GetState() != idSession::INGAME && sequence != -1) {
					//	int seq = peers[peerNum].snapProc->GetLastAppendedSequence();

					//	// When we aren't in the game, we need to send this as reliable msg's, since usercmds won't be taking care of it for us
					//	QueueReliableMessage(host, RELIABLE_SNAPSHOT_ACK, ackbuffer, sizeof(ackbuffer));
					//}
				}

				if (fullSnap) {
					sessionCB->ReceivedFullSnap();
					common->NetReceiveSnapshot(localSnap);
				}
			}
		}
	}

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
	idassert_ENUM_STRING(lobbyState_t::STATE_IDLE, 0),
	idassert_ENUM_STRING(lobbyState_t::STATE_CREATE_LOBBY_BACKEND, 1),
	idassert_ENUM_STRING(lobbyState_t::STATE_SEARCHING, 2),
	idassert_ENUM_STRING(lobbyState_t::STATE_OBTAINING_ADDRESS, 3),
	idassert_ENUM_STRING(lobbyState_t::STATE_CONNECT_HELLO_WAIT, 4),
	idassert_ENUM_STRING(lobbyState_t::STATE_FINALIZE_CONNECT, 5),
	idassert_ENUM_STRING(lobbyState_t::STATE_FAILED, 6),
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
	if (!idverify(lobbyBackend)) {
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	idassert(lobbyBackend);

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
idLobby::State_Searching
========================
*/
void idLobby::State_Searching() {
	if (!idverify(lobbyBackend)) {
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	if (lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_SEARCHING) {
		return;		// Busy but valid
	}

	if (lobbyBackend->GetState() != idLobbyBackend::lobbyBackendState_t::STATE_READY) {
		SetState(lobbyState_t::STATE_FAILED);		// Any other lobbyBackend state is invalid
		return;
	}

	// Done searching, get results from lobbyBackend
	lobbyBackend->GetSearchResults(searchResults);

	if (searchResults.empty()) {
		// If we didn't get any results, set state to failed
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	/*extern idCVar net_maxSearchResultsToTry;
	const int maxSearchResultsToTry = session->GetTitleStorageInt("net_maxSearchResultsToTry", net_maxSearchResultsToTry.GetInteger());

	if (searchResults.size() > maxSearchResultsToTry) {
		searchResults.SetNum(maxSearchResultsToTry);
	}*/

	// Set state to idle to signify we are done searching
	SetState(lobbyState_t::STATE_IDLE);
}

/*
========================
idLobby::State_Obtaining_Address
========================
*/
void idLobby::State_Obtaining_Address() {
	if (lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_OBTAINING_ADDRESS) {
		return;		// Valid but not ready
	}

	if (lobbyBackend->GetState() != idLobbyBackend::lobbyBackendState_t::STATE_READY) {
		// There was an error, signify to caller
		failedReason = failedReason_t::FAILED_CONNECT_FAILED;
		NET_VERBOSE_PRINT("idLobby::State_Obtaining_Address: the lobby backend failed.");
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	//
	//	We have the address of the lobbyBackend, we can now send a hello packet
	//

	// This will be the host for this lobby type
	host = AddPeer(hostAddress, GenerateSessionID());

	// Record start time of connection attempt to the host
	helloStartTime = Sys_Milliseconds();
	lastConnectRequest = helloStartTime;
	connectionAttempts = 0;

	// Change state to connecting
	SetState(lobbyState_t::STATE_CONNECT_HELLO_WAIT);

	// Send first connect attempt now (we'll send more periodically if we fail to receive an ack)
	// (we do this after changing state, since the function expects we're in the right state)
	SendConnectionRequest();
}

/*
========================
idLobby::State_Connect_Hello_Wait
========================
*/
void idLobby::State_Connect_Hello_Wait() {
	if (lobbyBackend->GetState() != idLobbyBackend::lobbyBackendState_t::STATE_READY) {
		// If the lobbyBackend is in an error state, shut everything down
		NET_VERBOSE_PRINT("NET: Lobby is no longer ready while waiting for lobbyType %s hello.\n", GetLobbyName().c_str());
		HandleConnectionAttemptFailed();
		return;
	}

	int time = Sys_Milliseconds();

	const int timeoutMs = net_connectTimeoutInSeconds.GetInteger() * 1000;

	if (timeoutMs != 0 && time - helloStartTime > timeoutMs) {
		NET_VERBOSE_PRINT("NET: Timeout waiting for lobbyType %s for party hello.\n", GetLobbyName().c_str());
		HandleConnectionAttemptFailed();
		return;
	}

	if (connectionAttempts < MAX_CONNECT_ATTEMPTS) {
		idassert(connectionAttempts >= 1);		// Should have at least the initial connection attempt

		// See if we need to send another hello request
		// (keep getting more frequent to increase chance due to possible packet loss, but clamp to MIN_CONNECT_FREQUENCY seconds)
		// TODO: We could eventually make timing out a function of actual number of attempts rather than just plain time.
		int resendTime = std::max(MIN_CONNECT_FREQUENCY_IN_SECONDS, CONNECT_REQUEST_FREQUENCY_IN_SECONDS / connectionAttempts) * 1000;

		if (time - lastConnectRequest > resendTime) {
			SendConnectionRequest();
			lastConnectRequest = time;
		}
	}
}

/*
========================
idLobby::State_Finalize_Connect
========================
*/
void idLobby::State_Finalize_Connect() {
	if (lobbyBackend->GetState() == idLobbyBackend::lobbyBackendState_t::STATE_CREATING) {
		// Valid but busy
		return;
	}

	if (lobbyBackend->GetState() != idLobbyBackend::lobbyBackendState_t::STATE_READY) {
		// Any other state not valid, failed
		SetState(lobbyState_t::STATE_FAILED);
		return;
	}

	// Success
	SetState(lobbyState_t::STATE_IDLE);
}

/*
========================
idLobby::SetState
========================
*/
void idLobby::SetState(lobbyState_t newState) {
	idassert(static_cast<int>(newState) < static_cast<int>(lobbyState_t::NUM_STATES));
	idassert(static_cast<int>(state) < static_cast<int>(lobbyState_t::NUM_STATES));

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
	idassert(lobbyBackend == nullptr);
	idassert(state == lobbyState_t::STATE_IDLE);

	lobbyBackend = sessionCB->CreateLobbyBackend(parms, 0, (idLobbyBackend::lobbyBackendType_t)lobbyType);

	SetState(lobbyState_t::STATE_CREATE_LOBBY_BACKEND);
}

/*
========================
idLobby::FindPeer
========================
*/
int idLobby::FindPeer(const lobbyAddress_t& remoteAddress, idPacketProcessor::sessionId_t sessionID, bool ignoreSessionID) {

	bool connectionless = (sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_PARTY ||
		sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_GAME ||
		sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_GAME_STATE);

	if (connectionless && !ignoreSessionID) {
		return -1;		// This was meant to be connectionless. FindPeer is meant for connected (or connecting) peers
	}

	for (int p = 0; p < peers.size(); p++) {
		if (peers[p].GetConnectionState() == connectionState_t::CONNECTION_FREE) {
			continue;
		}

		if (peers[p].address.Compare(remoteAddress)) {
			if (connectionless && ignoreSessionID) {
				return p;
			}

			// Using a rolling check, so that we account for possible packet loss, and out of order issues
			if (IsPeer()) {
				idPacketProcessor::sessionId_t searchStart = peers[p].sessionID;

				//	// Since we only roll the code between matches, we should only need to look ahead a couple increments.
				//	// Worse case, if the stars line up, the client doesn't see the new sessionId, and times out, and gets booted.
				//	// This should be impossible though, since the timings won't be possible considering how long it takes to end the match, 
				//	// and restart, and then restart again.
				//	int numTries = 2;

				//	while (numTries-- > 0 && searchStart != sessionID) {
				//		searchStart = IncrementSessionID(searchStart);
				//		if (searchStart == sessionID) {
				//			idLib::Printf("NET: Rolling session ID check found new ID: %i\n", searchStart);
				//			if (peers[p].packetProc != NULL) {
				//				peers[p].packetProc->VerifyEmptyReliableQueue(RELIABLE_GAME_DATA, RELIABLE_DUMMY_MSG);
				//			}
				//			peers[p].sessionID = searchStart;
				//			break;
				//		}
				//	}
			}

			if (peers[p].sessionID != sessionID) {
				continue;
			}
			return p;
		}
	}
	return -1;
}

/*
========================
idLobby::FindFreePeer
========================
*/
int idLobby::FindFreePeer() const {

	// Return the first non active peer
	for (int p = 0; p < peers.size(); p++) {
		if (!peers[p].IsActive()) {
			return p;
		}
	}
	return -1;
}

/*
========================
idLobby::AddPeer
========================
*/
int idLobby::AddPeer(const lobbyAddress_t& remoteAddress, idPacketProcessor::sessionId_t sessionID) {
	// First, make sure we don't already have this peer
	int p = FindPeer(remoteAddress, sessionID);
	idassert(p == -1);		// When using session ID's, we SHOULDN'T find this remoteAddress/sessionID combo

	if (p == -1) {
		// If we didn't find the peer, we need to add a new one

		p = FindFreePeer();

		if (p == -1) {
			peer_t newPeer;
			peers.push_back(std::move(newPeer));
			p = peers.size() - 1;
		}

		peer_t& peer = peers[p];

		peer.ResetAllData();

		idassert(peer.connectionState == connectionState_t::CONNECTION_FREE);

		peer.address = remoteAddress;

		peer.sessionID = sessionID;

		NET_VERBOSE_PRINT("NET: Added peer %s at index %i\n", remoteAddress.ToString().c_str(), p);
	}
	else {
		NET_VERBOSE_PRINT("NET: Found peer %s at index %i\n", remoteAddress.ToString().c_str(), p);
	}

	SetPeerConnectionState(p, connectionState_t::CONNECTION_CONNECTING);

	if (lobbyType == GetActingGameStateLobbyType()) {
		// Reset various flags used in game mode
		peers[p].ResetMatchData();
	}

	return p;
}

/*
========================
idLobby::DisconnectPeerFromSession
========================
*/
void idLobby::DisconnectPeerFromSession(int p) {
	if (!idverify(IsHost())) {
		return;
	}

	peer_t& peer = peers[p];

	if (peer.GetConnectionState() != connectionState_t::CONNECTION_FREE) {
		SetPeerConnectionState(p, connectionState_t::CONNECTION_FREE);
	}
}

/*
========================
idLobby::SendGoodbye
========================
*/
void idLobby::SendGoodbye(const lobbyAddress_t& remoteAddress, bool wasFull) {

	NET_VERBOSE_PRINT("NET: Sending goodbye to %s for %s (wasFull = %i)\n", remoteAddress.ToString().c_str(), GetLobbyName().c_str(), wasFull);

	static const int NUM_REDUNDANT_GOODBYES = 1;

	int msgType = OOB_GOODBYE;

	if (wasFull) {
		msgType = OOB_GOODBYE_FULL;
	}

	for (int i = 0; i < NUM_REDUNDANT_GOODBYES; i++) {
		SendConnectionLess(remoteAddress, static_cast<std::byte>(msgType));
	}
}

/*
========================
idLobby::SetPeerConnectionState
========================
*/
void idLobby::SetPeerConnectionState(int p, connectionState_t newState, bool skipGoodbye) {

	if (!idverify(p >= 0 && p < peers.size())) {
		idLib::Printf("NET: SetPeerConnectionState invalid peer index %i\n", p);
		return;
	}

	peer_t& peer = peers[p];

	const lobbyType_t actingGameStateLobbyType = GetActingGameStateLobbyType();

	if (peer.GetConnectionState() == newState) {
		idLib::Printf("NET: SetPeerConnectionState: Peer already in state %i\n", newState);
		idassert(0);	// This case means something is most likely bad, and it's the programmers fault
		return;
	}

	if (newState == connectionState_t::CONNECTION_CONNECTING) {
		// We better be coming from a free connection state if we are trying to connect
		idassert(peer.GetConnectionState() == connectionState_t::CONNECTION_FREE);

		idassert(!peer.packetProc);
		peer.packetProc = std::make_unique<idPacketProcessor>();

		if (lobbyType == actingGameStateLobbyType) {
			idassert(peer.snapProc == NULL);
			peer.snapProc = std::make_unique<idSnapshotProcessor>();
		}
	}
	else if (newState == connectionState_t::CONNECTION_ESTABLISHED) {
		// If we are marking this peer as connected for the first time, make sure this peer was actually trying to connect.
		idassert(peer.GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
	}
	else if (newState == connectionState_t::CONNECTION_FREE) {
		// If we are freeing this connection and we had an established connection before, make sure to send a goodbye
		if (peer.GetConnectionState() == connectionState_t::CONNECTION_ESTABLISHED && !skipGoodbye) {
			idLib::Printf("SetPeerConnectionState: Sending goodbye to peer %s from session %s\n", peer.address.ToString().c_str(), GetLobbyName().c_str());
			SendGoodbye(peer.address);
		}
	}

	peer.connectionState = newState;

	if (!peer.IsActive()) {
		if (peer.packetProc) {
			peer.packetProc = nullptr;
		}
	}

	// Do this in case we disconnected the peer
	if (IsHost()) {
		RemoveUsersWithDisconnectedPeers();
	}
}

/*
========================
idLobby::QueueReliableMessage
========================
*/
void idLobby::QueueReliableMessage(int p, reliableType_t type, const std::byte* data, int dataLen) {
	if (!idverify(p >= 0 && p < peers.size())) {
		return;
	}

	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {
		// Don't send to this peer if we don't have an established connection of this session type
		NET_VERBOSE_PRINT("NET: Not sending reliable type %i to peer %i because connectionState is %i\n", type, p, peer.GetConnectionState());
		return;
	}

	if (peer.packetProc->NumQueuedReliables() > 2) {
		idLib::Printf( "NET: peer.packetProc->NumQueuedReliables() > 2: %i (%i / %s)\n", peer.packetProc->NumQueuedReliables(), p, peer.address.ToString().c_str());
	}

	if (!peer.packetProc->QueueReliableMessage(static_cast<std::byte>(type), data, dataLen)) {
		// For now, when this happens, disconnect from all session types
		NET_VERBOSE_PRINT("NET: Dropping peer because we overflowed his reliable message queue\n");
		if (IsHost()) {
			// Disconnect peer from this session type
			DisconnectPeerFromSession(p);
		}
		else {
			Shutdown();		// Shutdown session if we can't queue the reliable
		}
	}
}

/*
========================
idLobby::SendConnectionLess
========================
*/
//void idLobby::SendConnectionLess(const lobbyAddress_t& remoteAddress, char type, const std::byte* data, int dataLen) {
//
//	const bool useDirectPort = (lobbyType == lobbyType_t::TYPE_GAME_STATE);
//
//	// Send it
//	sessionCB->SendRawPacket(remoteAddress, data, dataLen, useDirectPort);
//}

/*
========================
idLobby::SendConnectionLess
========================
*/
void idLobby::SendConnectionLess(const lobbyAddress_t& remoteAddress, std::byte type, const std::byte* data, int dataLen) {
	idBitMsg msg(data, dataLen);
	msg.SetSize(dataLen);

	std::array<std::byte, idPacketProcessor::MAX_OOB_MSG_SIZE> buffer;
	idBitMsg processedMsg(buffer.data(), sizeof(buffer));

	// Process the send
	idPacketProcessor::ProcessConnectionlessOutgoing(msg, processedMsg, static_cast<int>(lobbyType), static_cast<int>(type));

	const bool useDirectPort = (lobbyType == lobbyType_t::TYPE_GAME_STATE);

	// Send it
	sessionCB->SendRawPacket(remoteAddress, processedMsg.GetReadData(), processedMsg.GetSize(), useDirectPort);
}

/*
========================
idLobby::SendConnectionRequest
========================
*/
void idLobby::SendConnectionRequest() {
	// Some sanity checking
	idassert(state == lobbyState_t::STATE_CONNECT_HELLO_WAIT);
	idassert(peers[host].GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
	idassert(GetNumLobbyUsers() == 0);

	// Buffer to hold connect msg
	std::array<std::byte, idPacketProcessor::MAX_PACKET_SIZE - 2> buffer;
	idBitMsg msg(buffer.data(), sizeof(buffer));

	Serialize::ConnectionRequest proto_msg;
	proto_msg.set_sessionid(peers[host].sessionID);

	// We use InitSessionUsersFromLocalUsers here to copy the current local users over to session users simply to have a list
	// to send on the initial connection attempt.  We immediately clear our session user list once sent.
	InitSessionUsersFromLocalUsers(true);

	if (GetNumLobbyUsers() > 0) {
		for (int u = 0; u < GetNumLobbyUsers(); u++) {
			auto lobbyUser = GetLobbyUser(u);
			Serialize::Address address;
			address.set_port(lobbyUser->address.netAddr.port);
			*proto_msg.mutable_address() = address;
		}
	}
	else {
		FreeAllUsers();
		SetState(lobbyState_t::STATE_FAILED);

		return;
	}

	// We just used these users to fill up the msg above, we will get the real list from the server if we connect.
	FreeAllUsers();

	NET_VERBOSE_PRINT("NET: Sending hello to: %s (lobbyType: %s, session ID %i, attempt: %i)\n", hostAddress.ToString().c_str(), GetLobbyName().c_str(),
		peers[host].sessionID, connectionAttempts);

	msg.WriteProtobufMessage(&proto_msg);

	SendConnectionLess(hostAddress, static_cast<std::byte>(OOB_HELLO), msg.GetReadData(), msg.GetSize());

	connectionAttempts++;
}

/*
========================
idLobby::ConnectTo

Fires off a request to get the address of a lobbyBackend owner, and then attempts to connect (eventually handled in HandleObtainingLobbyOwnerAddress)
========================
*/
void idLobby::ConnectTo(const lobbyConnectInfo_t& connectInfo, bool fromInvite) {
	NET_VERBOSE_PRINT("NET: idSessionLocal::ConnectTo: fromInvite = %i\n", fromInvite);

	// Make sure current session is shutdown
	Shutdown();

	//connectIsFromInvite = fromInvite;

	lobbyBackend = sessionCB->JoinFromConnectInfo(connectInfo, (idLobbyBackend::lobbyBackendType_t)lobbyType);

	// First, we need the address of the lobbyBackend owner
	lobbyBackend->GetOwnerAddress(hostAddress);

	SetState(lobbyState_t::STATE_OBTAINING_ADDRESS);
}

/*
========================
idLobby::HandleGoodbyeFromPeer
========================
*/
void idLobby::HandleGoodbyeFromPeer(int peerNum, lobbyAddress_t& remoteAddress, int msgType) {
	if (peerNum < 0) {
		NET_VERBOSE_PRINT("NET: Goodbye from unknown peer %s on session %s\n", remoteAddress.ToString().c_str(), GetLobbyName().c_str());
		return;
	}

	if (peers[peerNum].GetConnectionState() == connectionState_t::CONNECTION_FREE) {
		NET_VERBOSE_PRINT("NET: Goodbye from peer %s on session %s that is not connected\n", remoteAddress.ToString().c_str(), GetLobbyName().c_str());
		return;
	}

	if (IsHost()) {
		// Goodbye from peer, remove him
		NET_VERBOSE_PRINT("NET: Goodbye from peer %s, on session %s\n", remoteAddress.ToString().c_str(), GetLobbyName().c_str());
		DisconnectPeerFromSession(peerNum);
	}
	else {
		// Let session handler take care of this
		NET_VERBOSE_PRINT("NET: Goodbye from host %s, on session %s\n", remoteAddress.ToString().c_str(), GetLobbyName().c_str());
		sessionCB->GoodbyeFromHost(*this, peerNum, remoteAddress, msgType);
	}
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
idLobby::ConnectToNextSearchResult
========================
*/
bool idLobby::ConnectToNextSearchResult() {
	if (lobbyType != lobbyType_t::TYPE_GAME) {
		return false;		// Only game sessions use matchmaking searches
	}

	// End current session lobby (this WON'T free search results)
	Shutdown();

	if (searchResults.size() == 0) {
		return false;		// No more search results to connect to, give up
	}

	// Get next search result
	lobbyConnectInfo_t connectInfo = searchResults[0];

	// Remove this search result
	searchResults.erase(searchResults.begin());

	// If we are connecting to a game lobby, tell our party to connect to this lobby as well
	if (lobbyType == lobbyType_t::TYPE_GAME && sessionCB->GetPartyLobby().IsLobbyActive()) {
		sessionCB->GetPartyLobby().SendMembersToLobby(lobbyType, connectInfo, true);
	}

	// Attempt to connect the lobby
	ConnectTo(connectInfo, true);		// Pass in true for invite, since searches are for matchmaking, and we should always be able to connect to those types of matches

	return true;	// Notify caller we are attempting to connect
}

/*
========================
idLobby::HandleInitialPeerConnection
Received on an initial peer connect request (OOB_HELLO)
========================
*/
int idLobby::HandleInitialPeerConnection(idBitMsg& msg, const lobbyAddress_t& peerAddress, int peerNum) {
	if (!IsHost()) {
		NET_VERBOSE_PRINT("NET: Got connectionless hello from peer %s on session, and we are not a host\n", peerAddress.ToString().c_str());
		SendGoodbye(peerAddress);
		return -1;
	}

	//if (!MatchTypeIsJoinInProgress(parms.matchFlags) && lobbyType == lobbyType_t::TYPE_GAME) {
	//	// No matter what, don't let people join migrated game sessions that are going to continue on to the same game
	//	// Not on invite list in a migrated game session - bounce him
	//	NET_VERBOSE_PRINT("NET: Denying game connection from %s since not on migration invite list\n", peerAddress.ToString().c_str());
	//	SendGoodbye(peerAddress);
	//	return -1;
	//}


	if (true) {
		// If this is for a game connection, make sure we have a game lobby
		if ((lobbyType == lobbyType_t::TYPE_GAME || lobbyType == lobbyType_t::TYPE_GAME_STATE) && sessionCB->GetState() < idSession::sessionState_t::GAME_LOBBY) {
			NET_VERBOSE_PRINT("NET: Denying game connection from %s because we don't have a game lobby\n", peerAddress.ToString().c_str());
			SendGoodbye(peerAddress);
			return -1;
		}
	}
	else {
		// If this is for a game connection, make sure we are in the game lobby
		if (lobbyType == lobbyType_t::TYPE_GAME && sessionCB->GetState() != idSession::sessionState_t::GAME_LOBBY) {
			NET_VERBOSE_PRINT("NET: Denying game connection from %s while not in game lobby\n", peerAddress.ToString().c_str());
			SendGoodbye(peerAddress);
			return -1;
		}
	}

	/*if (!CheckVersion(msg, peerAddress)) {
		idLib::Printf("NET: Denying user %s with wrong version number\n", peerAddress.ToString());
		SendGoodbye(peerAddress);
		return -1;
	}*/

	Serialize::ConnectionRequest proto_msg;
	if (!msg.ReadProtobufMessage(&proto_msg)) {
		return -1;
	}

	// Check to see if this is a peer trying to connect with a different sessionID
	// If the peer got abruptly disconnected, the peer could be trying to reconnect from a non clean disconnect
	if (peerNum >= 0) {
		peer_t& existingPeer = peers[peerNum];

		idassert(existingPeer.GetConnectionState() != connectionState_t::CONNECTION_FREE);

		if (existingPeer.sessionID == proto_msg.sessionid()) {
			return peerNum;		// If this is the same sessionID, then assume redundant connection attempt
		}

		//
		// This peer must be trying to reconnect from a previous abrupt disconnect
		//

		NET_VERBOSE_PRINT("NET: Reconnecting peer %s for session %s\n", peerAddress.ToString().c_str(), GetLobbyName().c_str());

		// Assume a peer is trying to reconnect from a non clean disconnect
		// We want to set the connection back to FREE manually, so we don't send a goodbye
		existingPeer.connectionState = connectionState_t::CONNECTION_FREE;

		if (existingPeer.packetProc) {
			existingPeer.packetProc = nullptr;
		}

		if (existingPeer.snapProc != NULL) {
			idassert(lobbyType == lobbyType_t::TYPE_GAME);		// Only games sessions should be creating snap processors
			existingPeer.snapProc = nullptr;
		}

		RemoveUsersWithDisconnectedPeers();

		peerNum = -1;
	}

	// Calling AddPeer will set our connectionState to this peer as CONNECTION_CONNECTING (which will get set to CONNECTION_ESTABLISHED below)
	peerNum = AddPeer(peerAddress, proto_msg.sessionid());

	peer_t& newPeer = peers[peerNum];

	idassert(newPeer.GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
	idassert(lobbyType != GetActingGameStateLobbyType() || newPeer.snapProc);

	// First, add users from this new peer to our user list 
	// (which will then forward the list to all peers except peerNum)
	//AddUsersFromMsg(msg, peerNum);

	// Mark the peer as connected for this session type
	SetPeerConnectionState(peerNum, connectionState_t::CONNECTION_ESTABLISHED);

	std::array<std::byte, idPacketProcessor::MAX_PACKET_SIZE> buffer;
	idBitMsg outmsg(buffer.data(), sizeof(buffer));

	Serialize::ReliableHello proto_out_msg;
	proto_out_msg.set_peernum(peerNum);

	Serialize::MatchParams proto_match_params;
	proto_match_params.set_gamemap(parms.gameMap);
	proto_match_params.set_matchflags(parms.matchFlags);
	proto_match_params.set_mapname(parms.mapName);

	*proto_out_msg.mutable_matchparams() = proto_match_params;

	outmsg.WriteProtobufMessage(&proto_out_msg);

	NET_VERBOSE_PRINT("NET: Sending response to %s, lobbyType %s, sessionID %i\n", peerAddress.ToString().c_str(), GetLobbyName().c_str(), 0);

	QueueReliableMessage(peerNum, reliableType_t::RELIABLE_HELLO, outmsg.GetReadData(), outmsg.GetSize());

	if (true/*MatchTypeIsJoinInProgress( parms.matchFlags )*/) {
		// We are are ingame, then start the client loading immediately
		if ((lobbyType == lobbyType_t::TYPE_GAME || lobbyType == lobbyType_t::TYPE_GAME_STATE) && sessionCB->GetState() >= idSession::sessionState_t::LOADING) {
			idLib::Printf("******* JOIN IN PROGRESS ********\n");
			if (sessionCB->GetState() == idSession::sessionState_t::INGAME) {
				//newPeer.pauseSnapshots = true;		// Since this player joined in progress, let game dictate when to start sending snaps
			}
			QueueReliableMessage(peerNum, idLobby::reliableType_t::RELIABLE_START_LOADING);
		}
	}

	return peerNum;
}

/*
========================
idLobby::InitStateLobbyHost
========================
*/
void idLobby::InitStateLobbyHost() {
	idassert(lobbyBackend);

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
idLobby::SendMembersToLobby
========================
*/
void idLobby::SendMembersToLobby(lobbyType_t destLobbyType, const lobbyConnectInfo_t& connectInfo, bool waitForOtherMembers) {

	// It's not our job to send party members to a game if we aren't the party host
	if (!IsHost()) {
		return;
	}

	// Send the message to all connected peers
	for (int i = 0; i < peers.size(); i++) {
		if (peers[i].IsConnected()) {
			SendPeerMembersToLobby(i, destLobbyType, connectInfo, waitForOtherMembers);
		}
	}
}

/*
========================
idLobby::SendMembersToLobby
========================
*/
void idLobby::SendMembersToLobby(idLobby& destLobby, bool waitForOtherMembers) {
	if (!destLobby.lobbyBackend) {
		return;		// We don't have a game lobbyBackend to get an address for
	}

	lobbyConnectInfo_t connectInfo = destLobby.lobbyBackend->GetConnectInfo();

	SendMembersToLobby(destLobby.lobbyType, connectInfo, waitForOtherMembers);
}

/*
========================
idLobby::SendPeerMembersToLobby
Give the address of a game lobby to a particular peer, notifying that peer to send a hello to the same server.
========================
*/
void idLobby::SendPeerMembersToLobby(int peerIndex, lobbyType_t destLobbyType, const lobbyConnectInfo_t& connectInfo, bool waitForOtherMembers) {
	// It's not our job to send party members to a game if we aren't the party host
	if (!IsHost()) {
		return;
	}

	idassert(peerIndex >= 0);
	idassert(peerIndex < peers.size());
	peer_t& peer = peers[peerIndex];

	NET_VERBOSE_PRINT("NET: Sending peer %i (%s) to game lobby\n", peerIndex, peer.address.ToString().c_str());

	if (!peer.IsConnected()) {
		idLib::Warning("NET: Can't send peer %i to game lobby: peer isn't in party", peerIndex);
		return;
	}
}

/*
========================
idLobby::EncodeSessionID
========================
*/
idPacketProcessor::sessionId_t idLobby::EncodeSessionID(uint32_t key) const {
	idassert(sizeof(uint32_t) >= sizeof(idPacketProcessor::sessionId_t));
	const int numBits = sizeof(idPacketProcessor::sessionId_t) * 8 - idPacketProcessor::NUM_LOBBY_TYPE_BITS;
	const uint32_t mask = (1 << numBits) - 1;
	idPacketProcessor::sessionId_t sessionID = (key & mask) << idPacketProcessor::NUM_LOBBY_TYPE_BITS;
	sessionID |= (static_cast<int>(lobbyType) + 1);
	return sessionID;
}

/*
========================
idLobby::EncodeSessionID
========================
*/
void idLobby::DecodeSessionID(idPacketProcessor::sessionId_t sessionID, uint32_t& key) const {
	idassert(sizeof(uint32_t) >= sizeof(idPacketProcessor::sessionId_t));
	key = sessionID >> idPacketProcessor::NUM_LOBBY_TYPE_BITS;
}

/*
========================
idLobby::GenerateSessionID
========================
*/
idPacketProcessor::sessionId_t idLobby::GenerateSessionID() const {
	idPacketProcessor::sessionId_t sessionID = EncodeSessionID(Sys_Milliseconds());

	// Make sure we can use it
	while (!SessionIDCanBeUsedForInBand(sessionID)) {
		sessionID = IncrementSessionID(sessionID);
	}

	return sessionID;
}

/*
========================
idLobby::SessionIDCanBeUsedForInBand
========================
*/
bool idLobby::SessionIDCanBeUsedForInBand(idPacketProcessor::sessionId_t sessionID) const {
	if (sessionID == idPacketProcessor::SESSION_ID_INVALID) {
		return false;
	}

	if (sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_PARTY) {
		return false;
	}

	if (sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_GAME) {
		return false;
	}

	if (sessionID == idPacketProcessor::SESSION_ID_CONNECTIONLESS_GAME_STATE) {
		return false;
	}

	return true;
}

/*
========================
idLobby::IncrementSessionID
========================
*/
idPacketProcessor::sessionId_t idLobby::IncrementSessionID(idPacketProcessor::sessionId_t sessionID) const {
	// Increment, taking into account valid id's
	while (1) {
		uint32_t key = 0;

		DecodeSessionID(sessionID, key);

		key++;

		sessionID = EncodeSessionID(key);

		if (SessionIDCanBeUsedForInBand(sessionID)) {
			break;
		}
	}

	return sessionID;
}

#define VERIFY_CONNECTED_PEER( p, sessionType_, msgType )				\
	if ( !idverify( lobbyType == sessionType_ ) ) {						\
		idLib::Printf( "NET: " #msgType ", peer:%s invalid session type for " #sessionType_ " %i.\n", peer.address.ToString(), sessionType_ );	\
		return;															\
	}																	\
	if ( peers[p].GetConnectionState() != connectionState_t::CONNECTION_ESTABLISHED ) {	\
		idLib::Printf( "NET: " #msgType ", peer:%s not connected for " #sessionType_ " %i.\n", peer.address.ToString(), sessionType_ );	\
		return;															\
	}

#define VERIFY_CONNECTING_PEER( p, sessionType_, msgType )				\
	if ( !idverify( lobbyType == sessionType_ ) ) {						\
		idLib::Printf( "NET: " #msgType ", peer:%s invalid session type for " #sessionType_ " %i.\n", peer.address.ToString(), sessionType_ );	\
		return;															\
	}																	\
	if ( peers[p].GetConnectionState() != connectionState_t::CONNECTION_CONNECTING ) {		\
		idLib::Printf( "NET: " #msgType ", peer:%s not connecting for " #sessionType_ " %i.\n", peer.address.ToString(), sessionType_ );	\
		return;															\
	}

#define VERIFY_FROM_HOST( p, sessionType_, msgType )					\
	VERIFY_CONNECTED_PEER( p, sessionType_, msgType );					\
	if ( p != host ) {													\
		idLib::Printf( "NET: "#msgType", not from "#sessionType_" host: %s\n", peer.address.ToString() );	\
		return;															\
	}																	\

#define VERIFY_FROM_CONNECTING_HOST( p, sessionType_, msgType )			\
	VERIFY_CONNECTING_PEER( p, sessionType_, msgType );					\
	if ( p != host ) {													\
		idLib::Printf( "NET: "#msgType", not from "#sessionType_" host: %s\n", peer.address.ToString() );	\
		return;															\
	}																	\

/*
========================
idLobby::HandleHelloAck
========================
*/
void idLobby::HandleHelloAck(int p, idBitMsg& msg) {
	peer_t& peer = peers[p];

	if (state != lobbyState_t::STATE_CONNECT_HELLO_WAIT) {
		idLib::Printf("NET: Hello ack for session type %s while not waiting for hello.\n", GetLobbyName().c_str());
		SendGoodbye(peer.address);		// We send a customary goodbye to make sure we are not in their list anymore
		return;
	}
	if (p != host) {
		// This shouldn't be possible
		idLib::Printf("NET: Hello ack for session type %s, not from correct host.\n", GetLobbyName());
		SendGoodbye(peer.address);		// We send a customary goodbye to make sure we are not in their list anymore
		return;
	}

	idassert(GetNumLobbyUsers() == 0);

	NET_VERBOSE_PRINT("NET: Hello ack for session type %s from %s\n", GetLobbyName().c_str(), peer.address.ToString().c_str());

	// We are now connected to this session type
	SetPeerConnectionState(p, connectionState_t::CONNECTION_ESTABLISHED);

	Serialize::ReliableHello proto_reliablehello;
	msg.ReadProtobufMessage(&proto_reliablehello);

	// Obtain what our peer index is on the host is
	peerIndexOnHost = proto_reliablehello.peernum();

	parms.gameMap = proto_reliablehello.matchparams().gamemap();
	parms.matchFlags = proto_reliablehello.matchparams().matchflags();
	parms.mapName = proto_reliablehello.matchparams().mapname();

	// Update lobbyBackend with parms
	/*if (lobbyBackend != NULL) {
		lobbyBackend->UpdateMatchParms(parms);
	}*/

	// Populate the user list with the one from the host (which will also include our local users)
	// This ensures the user lists are kept in sync
	FreeAllUsers();
	//AddUsersFromMsg(msg, p);

	// Make sure the host has a current heartbeat
	//peer.lastHeartBeat = Sys_Milliseconds();

	// Tell the lobby controller to finalize the connection
	SetState(lobbyState_t::STATE_FINALIZE_CONNECT);

	//
	// Success - We've received an ack from the server, letting us know we've been registered with the lobbies
	//
}

/*
========================
idLobby::HandleReliableMsg
========================
*/
void idLobby::HandleReliableMsg(int p, idBitMsg& msg) {
	peer_t& peer = peers[p];

	reliableType_t reliableType = static_cast<reliableType_t>(msg.ReadByte());

	idLib::Printf("NET: Received reliable msg type: %i \n", static_cast<int>(reliableType));

	const lobbyType_t actingGameStateLobbyType = GetActingGameStateLobbyType();

	if (reliableType == reliableType_t::RELIABLE_HELLO) {
		VERIFY_FROM_CONNECTING_HOST(p, lobbyType, RELIABLE_HELLO);
		// This is sent from the host acking a request to join the game lobby
		HandleHelloAck(p, msg);
		return;
	}
	//else if (reliableType == reliableType_t::RELIABLE_USER_CONNECT_REQUEST) {
	//	VERIFY_CONNECTED_PEER(p, lobbyType, RELIABLE_USER_CONNECT_REQUEST);

	//	// This message is sent from a peer requesting for a new user to join the game lobby
	//	// This will be sent while we are in a game lobby as a host.  otherwise, denied.
	//	NET_VERBOSE_PRINT("NET: RELIABLE_USER_CONNECT_REQUEST (%s) from %s\n", GetLobbyName().c_str(), peer.address.ToString().c_str());

	//	idSession::sessionState_t expectedState = (lobbyType == lobbyType_t::TYPE_PARTY) ? idSession::sessionState_t::PARTY_LOBBY : idSession::sessionState_t::GAME_LOBBY;

	//	if (sessionCB->GetState() == expectedState && IsHost() && NumFreeSlots() > 0) {	// This assumes only one user in the msg
	//		// Add user to session, which will also forward the operation to all other peers
	//		AddUsersFromMsg(msg, p);
	//	}
	//	else {
	//		// Let peer know user couldn't be added
	//		HandleUserConnectFailure(p, msg, RELIABLE_USER_CONNECT_DENIED);
	//	}
	//}
	//else if (reliableType == RELIABLE_USER_CONNECT_DENIED) {
	//	// This message is sent back from the host when a RELIABLE_PARTY_USER_CONNECT_REQUEST failed
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_PARTY_USER_CONNECT_DENIED);

	//	// Remove this user from the sign-in manager, so we don't keep trying to add them
	//	if (!sessionCB->GetSignInManager().RemoveLocalUserByHandle(localUserHandle_t(msg.ReadLong()))) {
	//		NET_VERBOSE_PRINT("NET: RELIABLE_PARTY_USER_CONNECT_DENIED, local user not found\n");
	//		return;
	//	}
	//}
	//else if (reliableType == RELIABLE_KICK_PLAYER) {
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_KICK_PLAYER);
	//	common->Dialog().AddDialog(GDM_KICKED, DIALOG_ACCEPT, NULL, NULL, false);
	//	if (sessionCB->GetPartyLobby().IsHost()) {
	//		session->SetSessionOption(idSession::OPTION_LEAVE_WITH_PARTY);
	//	}
	//	session->Cancel();
	//}
	//else if (reliableType == RELIABLE_HEADSET_STATE) {
	//	HandleHeadsetStateChange(p, msg);
	//}
	//else if (reliableType == RELIABLE_USER_CONNECTED) {
	//	// This message is sent back from the host when users have connected, and we need to update our lists to reflect that
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_USER_CONNECTED);

	//	NET_VERBOSE_PRINT("NET: RELIABLE_USER_CONNECTED (%s) from %s\n", GetLobbyName(), peer.address.ToString());
	//	AddUsersFromMsg(msg, p);
	//}
	//else if (reliableType == RELIABLE_USER_DISCONNECTED) {
	//	// This message is sent back from the host when users have diconnected, and we need to update our lists to reflect that
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_USER_DISCONNECTED);

	//	ProcessUserDisconnectMsg(msg);
	//}
	//else if (reliableType == RELIABLE_MATCH_PARMS) {
	//	parms.Read(msg);
	//	// Update lobby with parms
	//	if (lobbyBackend != NULL) {
	//		lobbyBackend->UpdateMatchParms(parms);
	//	}
	//}
	else if (reliableType == reliableType_t::RELIABLE_START_LOADING) {
		// This message is sent from the host to start loading a map
		VERIFY_FROM_HOST(p, actingGameStateLobbyType, RELIABLE_START_LOADING);

		NET_VERBOSE_PRINT("NET: RELIABLE_START_LOADING from %s\n", peer.address.ToString().c_str());

		startLoadingFromHost = true;
	}
	else if (reliableType == reliableType_t::RELIABLE_LOADING_DONE) {
		// This message is sent from the peers to state they are done loading the map
		VERIFY_CONNECTED_PEER(p, actingGameStateLobbyType, RELIABLE_LOADING_DONE);

		unsigned long networkChecksum = 0;
		networkChecksum = msg.ReadLong();

		//peer.networkChecksum = networkChecksum;
		peer.loaded = true;
	}
	else if (reliableType == reliableType_t::RELIABLE_IN_GAME) {
		VERIFY_CONNECTED_PEER(p, actingGameStateLobbyType, RELIABLE_IN_GAME);

		peer.inGame = true;
	}
	//else if (reliableType == RELIABLE_SNAPSHOT_ACK) {
	//	VERIFY_CONNECTED_PEER(p, actingGameStateLobbyType, RELIABLE_SNAPSHOT_ACK);

	//	// update our base state for his last received snapshot
	//	int snapNum = msg.ReadLong();
	//	float receivedBps = msg.ReadQuantizedUFloat< BANDWIDTH_REPORTING_MAX, BANDWIDTH_REPORTING_BITS >();

	//	// Update reported received bps
	//	if (peer.receivedBpsIndex != snapNum) {
	//		// Only do this the first time we get reported bps per snapshot. Subsequent ACKs of the same shot will usually have lower reported bps
	//		// due to more time elapsing but not receiving a new ss
	//		peer.receivedBps = receivedBps;
	//		peer.receivedBpsIndex = snapNum;
	//	}

	//	ApplySnapshotDelta(p, snapNum);

	//	//idLib::Printf( "NET: Peer %d Ack'd snapshot %d\n", p, snapNum );
	//	NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("NET: Peer %d Ack'd snapshot %d\n", p, snapNum));

	//}
	//else if (reliableType == RELIABLE_RESOURCE_ACK) {
	//}
	//else if (reliableType == RELIABLE_UPDATE_MATCH_PARMS) {
	//	VERIFY_CONNECTED_PEER(p, TYPE_GAME, RELIABLE_UPDATE_MATCH_PARMS);
	//	int msgType = msg.ReadLong();
	//	sessionCB->HandlePeerMatchParamUpdate(p, msgType);

	//}
	//else if (reliableType == RELIABLE_MATCHFINISHED) {
	//	VERIFY_FROM_HOST(p, actingGameStateLobbyType, RELIABLE_MATCHFINISHED);

	//	sessionCB->ClearMigrationState();

	//}
	//else if (reliableType == RELIABLE_ENDMATCH) {
	//	VERIFY_FROM_HOST(p, actingGameStateLobbyType, RELIABLE_ENDMATCH);

	//	sessionCB->EndMatchInternal();

	//}
	//else if (reliableType == RELIABLE_ENDMATCH_PREMATURE) {
	//	VERIFY_FROM_HOST(p, actingGameStateLobbyType, RELIABLE_ENDMATCH_PREMATURE);

	//	sessionCB->EndMatchInternal(true);

	//}
	//else if (reliableType == RELIABLE_START_MATCH_GAME_LOBBY_HOST) {
	//	// This message should be from the host of the game lobby, telling us (as the host of the GameStateLobby) to start loading
	//	VERIFY_CONNECTED_PEER(p, TYPE_GAME_STATE, RELIABLE_START_MATCH_GAME_LOBBY_HOST);

	//	if (session->GetState() >= idSession::LOADING) {
	//		NET_VERBOSE_PRINT("NET: RELIABLE_START_MATCH_GAME_LOBBY_HOST already loading\n");
	//		return;
	//	}

	//	// Read match parms, and start loading
	//	parms.Read(msg);

	//	// Send these new match parms to currently connected peers
	//	SendMatchParmsToPeers();

	//	startLoadingFromHost = true;		// Hijack this flag
	//}
	//else if (reliableType == RELIABLE_ARBITRATE) {
	//	VERIFY_CONNECTED_PEER(p, TYPE_GAME, RELIABLE_ARBITRATE);
	//	// Host telling us to arbitrate
	//	// Set a flag to do this later, since the lobby may not be in a state where it can fulfil the request at the moment
	//	respondToArbitrate = true;
	//}
	//else if (reliableType == RELIABLE_ARBITRATE_OK) {
	//	VERIFY_CONNECTED_PEER(p, TYPE_GAME, RELIABLE_ARBITRATE_OK);

	//	NET_VERBOSE_PRINT("NET: Got an arbitration ok from %d\n", p);

	//	everyoneArbitrated = true;
	//	for (int i = 0; i < GetNumLobbyUsers(); i++) {
	//		lobbyUser_t* user = GetLobbyUser(i);
	//		if (!verify(user != NULL)) {
	//			continue;
	//		}
	//		if (user->peerIndex == p) {
	//			user->arbitrationAcked = true;
	//		}
	//		else if (!user->arbitrationAcked) {
	//			everyoneArbitrated = false;
	//		}
	//	}

	//	if (everyoneArbitrated) {
	//		NET_VERBOSE_PRINT("NET: Everyone says they registered for arbitration, verifying\n");
	//		lobbyBackend->Arbitrate();
	//		//sessionCB->EveryoneArbitrated();
	//		return;
	//	}
	//}
	//else if (reliableType == RELIABLE_POST_STATS) {
	//	VERIFY_FROM_HOST(p, actingGameStateLobbyType, RELIABLE_POST_STATS);
	//	sessionCB->RecvLeaderboardStats(msg);
	//}
	//else if (reliableType == RELIABLE_SESSION_USER_MODIFIED) {
	//	VERIFY_CONNECTED_PEER(p, lobbyType, RELIABLE_SESSION_USER_MODIFIED);
	//	UpdateSessionUserOnPeers(msg);

	//}
	//else if (reliableType == RELIABLE_UPDATE_SESSION_USER) {
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_UPDATE_SESSION_USER);
	//	HandleUpdateSessionUser(msg);
	//}
	//else if (reliableType == RELIABLE_CONNECT_AND_MOVE_TO_LOBBY) {
	//	VERIFY_FROM_HOST(p, lobbyType, RELIABLE_CONNECT_AND_MOVE_TO_LOBBY);

	//	NET_VERBOSE_PRINT("NET: RELIABLE_CONNECT_AND_MOVE_TO_LOBBY\n");

	//	if (IsHost()) {
	//		idLib::Printf("RELIABLE_CONNECT_AND_MOVE_TO_LOBBY: We are the host.\n");
	//		return;
	//	}

	//	// Get connection info
	//	lobbyConnectInfo_t connectInfo;
	//	connectInfo.ReadFromMsg(msg);

	//	const lobbyType_t	destLobbyType = (lobbyType_t)msg.ReadByte();
	//	const bool			waitForMembers = msg.ReadBool();

	//	idassert(destLobbyType > lobbyType);		// Make sure this is a proper transition (i.e. TYPE_PARTY moves to TYPE_GAME, TYPE_GAME moves to TYPE_GAME_STATE)

	//	sessionCB->ConnectAndMoveToLobby(destLobbyType, connectInfo, waitForMembers);
	//}
	//else if (reliableType == RELIABLE_PARTY_CONNECT_OK) {
	//	VERIFY_FROM_HOST(p, TYPE_PARTY, RELIABLE_PARTY_CONNECT_OK);
	//	if (!sessionCB->GetGameLobby().waitForPartyOk) {
	//		idLib::Printf("RELIABLE_PARTY_CONNECT_OK: Wasn't waiting for ok.\n");
	//	}
	//	sessionCB->GetGameLobby().waitForPartyOk = false;
	//}
	//else if (reliableType == RELIABLE_PARTY_LEAVE_GAME_LOBBY) {
	//	VERIFY_FROM_HOST(p, TYPE_PARTY, RELIABLE_PARTY_LEAVE_GAME_LOBBY);

	//	NET_VERBOSE_PRINT("NET: RELIABLE_PARTY_LEAVE_GAME_LOBBY\n");

	//	if (sessionCB->GetState() != idSession::GAME_LOBBY) {
	//		idLib::Printf("RELIABLE_PARTY_LEAVE_GAME_LOBBY: Not in a game lobby, ignoring.\n");
	//		return;
	//	}

	//	if (IsHost()) {
	//		idLib::Printf("RELIABLE_PARTY_LEAVE_GAME_LOBBY: Host of party, ignoring.\n");
	//		return;
	//	}

	//	sessionCB->LeaveGameLobby();
	//}
	//else if (IsReliablePlayerToPlayerType(reliableType)) {
	//	HandleReliablePlayerToPlayerMsg(p, msg, reliableType);
	//}
	//else if (reliableType == RELIABLE_PING) {
	//	HandleReliablePing(p, msg);
	//}
	//else if (reliableType == RELIABLE_PING_VALUES) {
	//	HandlePingValues(msg);
	//}
	//else if (reliableType == RELIABLE_BANDWIDTH_VALUES) {
	//	HandleBandwidhTestValue(p, msg);
	//}
	//else if (reliableType == RELIABLE_MIGRATION_GAME_DATA) {
	//	HandleMigrationGameData(msg);
	//}
	//else if (reliableType >= RELIABLE_GAME_DATA) {

	//	VERIFY_CONNECTED_PEER(p, lobbyType, RELIABLE_GAME_DATA);

	//	common->NetReceiveReliable(p, reliableType - RELIABLE_GAME_DATA, msg);
	//}
	//else if (reliableType == RELIABLE_DUMMY_MSG) {
	//	// Ignore dummy msg's
	//	NET_VERBOSE_PRINT("NET: ignoring dummy msg from %s\n", peer.address.ToString());
	//}
	//else {
	//	NET_VERBOSE_PRINT("NET: Unknown reliable packet type %d from %s\n", reliableType, peer.address.ToString());
	//}
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
idLobby::SendAnotherFragment
Other than connectionless sends, this should be the chokepoint for sending packets to peers.
========================
*/
bool idLobby::SendAnotherFragment(int p) {
	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {	// Not connected to any mode (party or game), so no need to send
		return false;
	}

	if (!peer.packetProc->HasMoreFragments()) {
		return false;		// No fragments to send for this peer
	}

	//if (!CanSendMoreData(p)) {
	//	return false;		// We need to throttle the sends so we don't saturate the connection
	//}

	int time = Sys_Milliseconds();

	//if (time - peer.lastFragmentSendTime < 2) {
	//	NET_VERBOSE_PRINT("Too soon to send another packet. Delta: %d \n", (time - peer.lastFragmentSendTime));
	//	return false;		// Too soon to send another fragment
	//}

	//peer.lastFragmentSendTime = time;

	bool sentFragment = false;

	while (true) {
		idBitMsg msg;
		// We use the final packet size here because it has been processed, and no more headers will be added
		std::array<std::byte, idPacketProcessor::MAX_FINAL_PACKET_SIZE> buffer;
		msg.InitWrite(buffer.data(), sizeof(buffer));

		if (!peers[p].packetProc->GetSendFragment(time, peers[p].sessionID, msg)) {
			break;
		}

		const bool useDirectPort = (lobbyType == lobbyType_t::TYPE_GAME_STATE);

		msg.BeginReading();
		sessionCB->SendRawPacket(peers[p].address, msg.GetReadData(), msg.GetSize(), useDirectPort);
		sentFragment = true;
		break;		// Comment this out to send all fragments in one burst
	}

	if (peer.packetProc->HasMoreFragments()) {
		NET_VERBOSE_PRINT("More packets left after ::SendAnotherFragment\n");
	}

	return sentFragment;
}

/*
========================
idLobby::ProcessOutgoingMsg
========================
*/
void idLobby::ProcessOutgoingMsg(int p, const void* data, int size, bool isOOB, int userData) {

	peer_t& peer = peers[p];

	if (peer.GetConnectionState() != connectionState_t::CONNECTION_ESTABLISHED) {
		idLib::Printf("peer.GetConnectionState() != CONNECTION_ESTABLISHED\n");
		return;	// Peer not fully connected for this session type, return
	}

	if (peer.packetProc->HasMoreFragments()) {
		idLib::Error("FATAL: Attempt to process a packet while fragments still need to be sent.\n"); // We can't handle this case
	}

	int currentTime = Sys_Milliseconds();

	// if ( currentTime - peer.lastProcTime < 30 ) {
	//	 idLib::Printf("ProcessOutgoingMsg called within %dms %s\n", (currentTime - peer.lastProcTime), GetLobbyName() );
	// }

	//peer.lastProcTime = currentTime;

	//if (!isOOB) {
	//	// Keep track of the last time an in-band packet was sent 
	//	// (used for things like knowing when reliables could have been last sent)
	//	peer.lastInBandProcTime = peer.lastProcTime;
	//}

	idBitMsg msg;
	msg.InitRead(static_cast<const std::byte*>(data), size);
	peer.packetProc->ProcessOutgoing(currentTime, msg, isOOB, userData);
}

/*
========================
idLobby::ResendReliables
========================
*/
void idLobby::ResendReliables(int p) {

	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {
		return;
	}

	if (peer.packetProc->HasMoreFragments()) {
		return;		// We can't send more data while fragments are still being sent out
	}

	/*if (!CanSendMoreData(p)) {
		return;
	}*/

	int time = Sys_Milliseconds();

	const int DEFAULT_MIN_RESEND = 20;		// Quicker resend while not in game to speed up resource transmission acks
	const int DEFAULT_MIN_RESEND_INGAME = 100;

	//int resendWait = DEFAULT_MIN_RESEND_INGAME;

	//if (sessionCB->GetState() == idSession::INGAME) {
	//	// setup some minimum waits and account for ping
	//	resendWait = Max(DEFAULT_MIN_RESEND_INGAME, peer.lastPingRtt / 2);
	//	if (lobbyType == TYPE_PARTY) {
	//		resendWait = Max(500, resendWait); // party session does not need fast frequency at all once in game
	//	}
	//}
	//else {
	//	// don't trust the ping when still loading stuff
	//	// need to resend fast to speed up transmission of network decls
	//	resendWait = DEFAULT_MIN_RESEND;
	//}

	//if (time - peer.lastInBandProcTime < resendWait) {
	//	// no need to resend reliables if they went out on an in-band packet recently
	//	return;
	//}

	if (peer.packetProc->NumQueuedReliables() > 0  || peer.packetProc->NeedToSendReliableAck()) {
		//NET_VERBOSE_PRINT( "NET: ResendReliables %s\n", GetLobbyName() );
		ProcessOutgoingMsg(p, nullptr, 0, false, 0);		// Force an empty unreliable msg so any reliables will get processed as well
	}
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

	// Resend reliable msg's (do this before we send out the fragments)
	for (int p = 0; p < peers.size(); p++) {
		ResendReliables(p);
	}

	// If we haven't sent anything to our peers in a long time, make sure to send an empty packet (so our heartbeat gets updated) so we don't get disconnected
	// NOTE - We used to only send these to the host, but the host needs to also send these to clients
	//for (int p = 0; p < peers.size(); p++) {
	//	if (!peers[p].IsConnected() || peers[p].packetProc->HasMoreFragments()) {
	//		continue;
	//	}
	//	if (newTime - peers[p].lastProcTime > 1000 * PEER_HEARTBEAT_IN_SECONDS) {
	//		//NET_VERBOSE_PRINT( "NET: ProcessOutgoing Heartbeat %s\n", GetLobbyName() );
	//		ProcessOutgoingMsg(p, NULL, 0, false, 0);
	//	}
	//}

	// Send any unsent fragments for each peer (do this last)
	for (int p = 0; p < peers.size(); p++) {
		SendAnotherFragment(p);
	}
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