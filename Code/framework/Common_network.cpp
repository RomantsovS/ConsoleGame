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
===============
idCommonLocal::NetReceiveSnapshot
===============
*/
void idCommonLocal::NetReceiveSnapshot(class idSnapShot& ss) {
	//ss.SetRecvTime(Sys_Milliseconds());
	// If we are about to overwrite the oldest snap, then force a read, which will cause a pop on screen, but we have to do this.
	if (writeSnapshotIndex - readSnapshotIndex >= RECEIVE_SNAPSHOT_BUFFER_SIZE) {
		idLib::Printf("Overwritting oldest snapshot %d with new snapshot %d\n", readSnapshotIndex, writeSnapshotIndex);
		idassert(writeSnapshotIndex % RECEIVE_SNAPSHOT_BUFFER_SIZE == readSnapshotIndex % RECEIVE_SNAPSHOT_BUFFER_SIZE);
		ProcessNextSnapshot();
	}

	receivedSnaps[writeSnapshotIndex % RECEIVE_SNAPSHOT_BUFFER_SIZE] = ss;
	writeSnapshotIndex++;

	// Force read the very first 2 snapshots
	if (readSnapshotIndex < 2) {
		ProcessNextSnapshot();
	}
}

/*
========================
idCommonLocal::ProcessSnapshot
========================
*/
void idCommonLocal::ProcessSnapshot(idSnapShot& ss) {
	int time = Sys_Milliseconds();

	/*snapTime = time;
	snapPrevious = snapCurrent;
	snapCurrent.serverTime = ss.GetTime();
	snapRate = snapCurrent.serverTime - snapPrevious.serverTime;


	static int lastReceivedLocalTime = 0;
	int timeSinceLastSnap = (time - lastReceivedLocalTime);
	if (net_debug_snapShotTime.GetBool()) {
		idLib::Printf("^2ProcessSnapshot. delta serverTime: %d  delta localTime: %d \n", (snapCurrent.serverTime - snapPrevious.serverTime), timeSinceLastSnap);
	}
	lastReceivedLocalTime = time;*/

	// Read usercmds from other players
	/*for (int p = 0; p < MAX_PLAYERS; p++) {
		if (p == game->GetLocalClientNum()) {
			continue;
		}
		idBitMsg msg;
		if (ss.GetObjectMsgByID(SNAP_USERCMDS + p, msg)) {
			NetReadUsercmds(p, msg);
		}
	}*/




	// Set server game time here so that it accurately reflects the time when this frame was saved out, in case any serialize function needs it.
	//int oldTime = Game()->GetServerGameTimeMs();
	//Game()->SetServerGameTimeMs(snapCurrent.serverTime);

	Game()->ClientReadSnapshot(ss); //, &oldss );

	// Restore server game time
	//Game()->SetServerGameTimeMs(oldTime);

	//snapTimeDelta = ss.GetRecvTime() - oldss.GetRecvTime();
	//oldss = ss;
}

/*
========================
idCommonLocal::ProcessNextSnapshot
========================
*/
void idCommonLocal::ProcessNextSnapshot() {
	if (readSnapshotIndex == writeSnapshotIndex) {
		idLib::Printf("No snapshots to process.\n");
		return;		// No snaps to process
	}
	ProcessSnapshot(receivedSnaps[readSnapshotIndex % RECEIVE_SNAPSHOT_BUFFER_SIZE]);
	readSnapshotIndex++;
}

/*
========================
idCommonLocal::RunNetworkSnapshotFrame
========================
*/
void idCommonLocal::RunNetworkSnapshotFrame() {
	ProcessNextSnapshot();
}

/*
========================
idCommonLocal::ResetNetworkingState
========================
*/
void idCommonLocal::ResetNetworkingState() {
	readSnapshotIndex = 0;
	writeSnapshotIndex = 0;

	userCmdMgr.SetDefaults();

	gameFrame = 0;
	//nextUsercmdSendTime = 0;
	nextSnapshotSendTime = 0;
}