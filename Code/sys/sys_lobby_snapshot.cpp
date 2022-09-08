#include "idlib/precompiled.h"
#include "sys_lobby.h"

/*
========================
idLobby::UpdateSnaps
========================
*/
void idLobby::UpdateSnaps() {
	assert(lobbyType == GetActingGameStateLobbyType());

	//haveSubmittedSnaps = false;

	if (!SendCompletedSnaps()) {
		// If we weren't able to send all the submitted snaps, we need to wait till we can.
		// We can't start new jobs until they are all sent out.
		return;
	}

	for (int p = 0; p < peers.size(); p++) {
		peer_t& peer = peers[p];

		if (!peer.IsConnected()) {
			continue;
		}

		if (peer.needToSubmitPendingSnap) {
			// Submit the snap
			if (SubmitPendingSnap(p)) {
				peer.needToSubmitPendingSnap = false;	// only clear this if we actually submitted the snap
			}

		}
	}
}

/*
========================
idLobby::SendCompletedSnaps
This function will send send off any previously submitted pending snaps if they are ready
========================
*/
bool idLobby::SendCompletedSnaps() {
	assert(lobbyType == GetActingGameStateLobbyType());

	bool sentAllSubmitted = true;

	for (int p = 0; p < peers.size(); p++) {
		peer_t& peer = peers[p];

		if (!peer.IsConnected()) {
			continue;
		}

		if (true/*peer.snapProc->PendingSnapReadyToSend()*/) {
			// Check to see if there are any snaps that were submitted that need to be sent out
			SendCompletedPendingSnap(p);
		}
		else if (IsHost()) {
			NET_VERBOSESNAPSHOT_PRINT_LEVEL(7, va("  ^8Peer %d pendingSnap not ready to send\n", p).c_str());
		}

		if (!peer.IsConnected()) { // peer may have been dropped in "SendCompletedPendingSnap". ugh.
			continue;
		}

		//if (peer.snapProc->PendingSnapReadyToSend()) {
		//	// If we still have a submitted snap, we know we're not done
		//	sentAllSubmitted = false;
		//	if (IsHost()) {
		//		NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("  ^2Peer %d did not send all submitted snapshots.\n", p).c_str());
		//	}
		//}
	}

	return sentAllSubmitted;
}

/*
========================
idLobby::SubmitPendingSnap
========================
*/
bool idLobby::SubmitPendingSnap(int p) {

	assert(lobbyType == GetActingGameStateLobbyType());

	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {
		return false;
	}

	// If the peer doesn't have the latest resource list, send it to him before sending any new snapshots
	/*if (SendResources(p)) {
		return false;
	}*/

	if (!peer.loaded) {
		return false;
	}

	if (!peer.snapProc->HasPendingSnap()) {
		return false;
	}

	int time = Sys_Milliseconds();

	/*int timeFromLastSub = time - peer.lastSnapJobTime;

	int forceResendTime = session->GetTitleStorageInt("net_snap_redundant_resend_in_ms", net_snap_redundant_resend_in_ms.GetInteger());

	if (timeFromLastSub < forceResendTime && peer.snapProc->IsBusyConfirmingPartialSnap()) {
		return false;
	}

	peer.lastSnapJobTime = time;
	assert(!peer.snapProc->PendingSnapReadyToSend());*/

	// Submit snapshot delta to jobs
	peer.snapProc->SubmitPendingSnap();

	//NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("  Submitted snapshot to jobList for peer %d. Since last jobsub: %d\n", p, timeFromLastSub));

	return true;
}

/*
========================
idLobby::SendCompletedPendingSnap
========================
*/
void idLobby::SendCompletedPendingSnap(int p) {

	assert(lobbyType == GetActingGameStateLobbyType());

	int time = Sys_Milliseconds();

	peer_t& peer = peers[p];

	if (!peer.IsConnected()) {
		return;
	}

	if (!peer.snapProc /* || !peer.snapProc->PendingSnapReadyToSend()*/) {
		return;
	}

	// If we have a pending snap ready to send, we better have a pending snap
	assert(peer.snapProc->HasPendingSnap());

	// Get the snap data blob now, even if we don't send it.  
	// This is somewhat wasteful, but we have to do this to keep the snap job pipe ready to keep doing work
	// If we don't do this, this peer will cause other peers to be starved of snapshots, when they may very well be ready to send a snap
	std::array<std::byte, MAX_SNAP_SIZE> buffer;
	int maxLength = sizeof(buffer) - peer.packetProc->GetReliableDataSize() - 128;

	int size = peer.snapProc->GetPendingSnapDelta(buffer.data(), maxLength);

	/*if (!CanSendMoreData(p)) {
		return;
	}*/

	// Can't send anymore snapshots until all fragments are sent
	if (peer.packetProc->HasMoreFragments()) {
		return;
	}

	// If the peer doesn't have the latest resource list, send it to him before sending any new snapshots
	/*if (SendResources(p)) {
		return;
	}*/

	//int timeFromJobSub = time - peer.lastSnapJobTime;
	//int timeFromLastSend = time - peer.lastSnapTime;

	//if (timeFromLastSend > 0) {
	//	peer.snapHz = 1000.0f / (float)timeFromLastSend;
	//}
	//else {
	//	peer.snapHz = 0.0f;
	//}

	//if (net_snapshot_send_warntime.GetInteger() > 0 && peer.lastSnapTime != 0 && net_snapshot_send_warntime.GetInteger() < timeFromLastSend) {
	//	idLib::Printf("NET: Took %d ms to send peer %d snapshot\n", timeFromLastSend, p);
	//}

	//if (peer.throttleSnapsForXSeconds != 0) {
	//	if (time < peer.throttleSnapsForXSeconds) {
	//		return;
	//	}

	//	// If we were trying to recover ping, see if we succeeded
	//	if (peer.recoverPing != 0) {
	//		if (peer.lastPingRtt >= peer.recoverPing) {
	//			peer.failedPingRecoveries++;
	//		}
	//		else {
	//			const int peer_throttle_minSnapSeq = session->GetTitleStorageInt("net_peer_throttle_minSnapSeq", net_peer_throttle_minSnapSeq.GetInteger());
	//			if (peer.snapProc->GetFullSnapBaseSequence() > idSnapshotProcessor::INITIAL_SNAP_SEQUENCE + peer_throttle_minSnapSeq) {
	//				// If throttling recovered the ping
	//				int maxRate = common->GetSnapRate() * session->GetTitleStorageInt("net_peer_throttle_maxSnapRate", net_peer_throttle_maxSnapRate.GetInteger());
	//				peer.throttledSnapRate = idMath::ClampInt(common->GetSnapRate(), maxRate, peer.throttledSnapRate + common->GetSnapRate());
	//			}
	//		}
	//	}

	//	peer.throttleSnapsForXSeconds = 0;
	//}

	//peer.lastSnapTime = time;

	if (size != 0) {
		if (size > 0) {
			/*NET_VERBOSESNAPSHOT_PRINT_LEVEL(3, va("NET: (peer %d) Sending snapshot %d delta'd against %d. Since JobSub: %d Since LastSend: %d. Size: %d\n", p,
				peer.snapProc->GetSnapSequence(), peer.snapProc->GetBaseSequence(), timeFromJobSub, timeFromLastSend, size).c_str());*/
			ProcessOutgoingMsg(p, buffer.data(), size, false, 0);
		}
		//else if (size < 0) {	// Size < 0 indicates the delta buffer filled up
		//	// There used to be code here that would disconnect peers if they were in game and filled up the buffer
		//	// This was causing issues in the playtests we were running (Doom 4 MP) and after some conversation
		//	// determined that it was not needed since a timeout mechanism has been added since
		//	ProcessOutgoingMsg(p, buffer, -size, false, 0);
		//	if (peer.snapProc != NULL) {
		//		NET_VERBOSESNAPSHOT_PRINT("NET: (peerNum: %d - name: %s) Resending last snapshot delta %d because his delta list filled up. Since JobSub: %d Since LastSend: %d Delta Size: %d\n", p, GetPeerName(p), peer.snapProc->GetSnapSequence(), timeFromJobSub, timeFromLastSend, size);
		//	}
		//}
	}
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

	if (peer.snapProc->TrySetPendingSnapshot(ss)) {
		NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("  ^8Set next pending snapshot peer %d\n", 0).c_str());

		/*peer.numSnapsSent++;

		idSnapShot* baseState = peers[p].snapProc->GetBaseState();
		if (verify(baseState != NULL)) {
			baseState->UpdateExpectedSeq(peers[p].snapProc->GetSnapSequence());
		}*/

	}
	else {
		NET_VERBOSESNAPSHOT_PRINT_LEVEL(2, va("  ^2FAILED Set next pending snapshot peer %d\n", 0).c_str());
	}

	// We send out the pending snap, which could be the most recent, or an old one that hasn't fully been sent
	// We don't send immediately, since we have to coordinate sending snaps for all peers in same place considering jobs.
	peer.needToSubmitPendingSnap = true;
}