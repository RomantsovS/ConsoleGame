#include "idlib/precompiled.h"
#include "sys_lobby.h"
#include <ConnectionRequest.pb.h>

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
idLobby::StartFinding
========================
*/
void idLobby::StartFinding(const idMatchParameters& parms_) {
	parms = parms_;

	Shutdown();		// Make sure we're in a shutdown state before proceeding

	assert(GetNumLobbyUsers() == 0);
	assert(!lobbyBackend);

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
		//case lobbyState_t::STATE_FINALIZE_CONNECT:		State_Finalize_Connect();		break;
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
idLobby::HandlePacket
========================
*/
void idLobby::HandlePacket(lobbyAddress_t& remoteAddress, idBitMsg& fragMsg, idPacketProcessor::sessionId_t sessionID) {
	idBitMsg msg;

	int peerNum = FindPeer(remoteAddress, sessionID);
	int type = idPacketProcessor::RETURN_TYPE_NONE;
	int	userData = 0;

	if (peerNum >= 0) {
		if (!peers[peerNum].IsActive()) {
			idLib::Printf("NET: Received in-band packet from peer %s with no active connection.\n", remoteAddress.ToString());
			return;
		}
	}
	else {
		if (!idPacketProcessor::ProcessConnectionlessIncoming(fragMsg, sessionID, msg, userData)) {
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
			peerNum = HandleInitialPeerConnection(fragMsg, remoteAddress, peerNum);
			return;
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
idLobby::State_Searching
========================
*/
void idLobby::State_Searching() {
	if (!verify(lobbyBackend)) {
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
		assert(connectionAttempts >= 1);		// Should have at least the initial connection attempt

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
	assert(p == -1);		// When using session ID's, we SHOULDN'T find this remoteAddress/sessionID combo

	if (p == -1) {
		// If we didn't find the peer, we need to add a new one

		p = FindFreePeer();

		if (p == -1) {
			peer_t newPeer;
			peers.push_back(newPeer);
			p = peers.size() - 1;
		}

		peer_t& peer = peers[p];

		peer.ResetAllData();

		assert(peer.connectionState == connectionState_t::CONNECTION_FREE);

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
	if (!verify(IsHost())) {
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

	for (int i = 0; i < NUM_REDUNDANT_GOODBYES; i++) {
		SendConnectionLess(remoteAddress, 0);
	}
}

/*
========================
idLobby::SetPeerConnectionState
========================
*/
void idLobby::SetPeerConnectionState(int p, connectionState_t newState, bool skipGoodbye) {

	if (!verify(p >= 0 && p < peers.size())) {
		idLib::Printf("NET: SetPeerConnectionState invalid peer index %i\n", p);
		return;
	}

	peer_t& peer = peers[p];

	const lobbyType_t actingGameStateLobbyType = GetActingGameStateLobbyType();

	if (peer.GetConnectionState() == newState) {
		idLib::Printf("NET: SetPeerConnectionState: Peer already in state %i\n", newState);
		assert(0);	// This case means something is most likely bad, and it's the programmers fault
		//assert((peer.packetProc != NULL) == peer.IsActive());
		//assert(((peer.snapProc != NULL) == peer.IsActive()) == (actingGameStateLobbyType == lobbyType));
		return;
	}

	if (newState == connectionState_t::CONNECTION_CONNECTING) {
		// We better be coming from a free connection state if we are trying to connect
		assert(peer.GetConnectionState() == connectionState_t::CONNECTION_FREE);
	}
	else if (newState == connectionState_t::CONNECTION_ESTABLISHED) {
		// If we are marking this peer as connected for the first time, make sure this peer was actually trying to connect.
		assert(peer.GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
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
void idLobby::QueueReliableMessage(int p, char type, const char* data, int dataLen) {
	if (!verify(p >= 0 && p < peers.size())) {
		return;
	}

	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {
		// Don't send to this peer if we don't have an established connection of this session type
		NET_VERBOSE_PRINT("NET: Not sending reliable type %i to peer %i because connectionState is %i\n", type, p, peer.GetConnectionState());
		return;
	}
}

/*
========================
idLobby::SendConnectionLess
========================
*/
void idLobby::SendConnectionLess(const lobbyAddress_t& remoteAddress, char type, const std::byte* data, int dataLen) {

	const bool useDirectPort = (lobbyType == lobbyType_t::TYPE_GAME_STATE);

	// Send it
	sessionCB->SendRawPacket(remoteAddress, data, dataLen, useDirectPort);
}

/*
========================
idLobby::SendConnectionLess
========================
*/
void idLobby::SendConnectionLess(const lobbyAddress_t& remoteAddress, boost::asio::streambuf& buf) {
	const bool useDirectPort = (lobbyType == lobbyType_t::TYPE_GAME_STATE);

	// Send it
	sessionCB->SendRawPacket(remoteAddress, buf, useDirectPort);
}

/*
========================
idLobby::SendConnectionRequest
========================
*/
void idLobby::SendConnectionRequest() {
	// Some sanity checking
	assert(state == lobbyState_t::STATE_CONNECT_HELLO_WAIT);
	assert(peers[host].GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
	assert(GetNumLobbyUsers() == 0);

	Serialize::ConnectionRequest proto_msg;
	proto_msg.set_sessionid(peers[host].sessionID);
	proto_msg.set_type(OOB_HELLO);

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

	boost::asio::streambuf buffer;
	idBitMsg msg;
	msg.InitWrite(buffer);

	// Process the send
	idPacketProcessor::ProcessConnectionlessOutgoing(msg, static_cast<int>(lobbyType));

	size_t size = proto_msg.ByteSizeLong();

	msg.WriteLongLong(size);

	if (!msg.WriteProtobufMessage(&proto_msg, size))
		return;
	
	msg.Flush();

	size = buffer.size();
	SendConnectionLess(hostAddress, buffer);

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

	// Calling AddPeer will set our connectionState to this peer as CONNECTION_CONNECTING (which will get set to CONNECTION_ESTABLISHED below)
	peerNum = AddPeer(peerAddress, proto_msg.sessionid());

	peer_t& newPeer = peers[peerNum];

	assert(newPeer.GetConnectionState() == connectionState_t::CONNECTION_CONNECTING);
	//assert(lobbyType != GetActingGameStateLobbyType() || newPeer.snapProc != NULL);

	// First, add users from this new peer to our user list 
	// (which will then forward the list to all peers except peerNum)
	//AddUsersFromMsg(msg, peerNum);

	// Mark the peer as connected for this session type
	SetPeerConnectionState(peerNum, connectionState_t::CONNECTION_ESTABLISHED);

	NET_VERBOSE_PRINT("NET: Sending response to %s, lobbyType %s, sessionID %i\n", peerAddress.ToString().c_str(), GetLobbyName().c_str(), 0);

	//QueueReliableMessage(peerNum, RELIABLE_HELLO, outmsg.GetReadData(), outmsg.GetSize());

	return peerNum;
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

	assert(peerIndex >= 0);
	assert(peerIndex < peers.size());
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
	assert(sizeof(uint32_t) >= sizeof(idPacketProcessor::sessionId_t));
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
	assert(sizeof(uint32_t) >= sizeof(idPacketProcessor::sessionId_t));
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