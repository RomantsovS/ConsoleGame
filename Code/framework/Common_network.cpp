#include "idlib/precompiled.h"

#include "Common_local.h"

idCVar net_snapRate("net_snapRate", "100", CVAR_SYSTEM | CVAR_INTEGER, "How many milliseconds between sending snapshots");

idCVar net_debug_snapShotTime("net_debug_snapShotTime", "1", CVAR_BOOL | CVAR_ARCHIVE, "");
idCVar net_debug_snapShot("net_debug_snapShot", "1", CVAR_BOOL | CVAR_ARCHIVE, "");
idCVar com_forceLatestSnap("com_forceLatestSnap", "0", CVAR_BOOL, "");

// Factor at which we catch speed up interpolation if we fall behind our optimal interpolation window
//  -This is a static factor. We may experiment with a dynamic one that would be faster the farther you are from the ideal window
idCVar net_interpolationCatchupRate("net_interpolationCatchupRate", "1.3", CVAR_FLOAT, "Scale interpolationg rate when we fall behind");
idCVar net_interpolationFallbackRate("net_interpolationFallbackRate", "0.95", CVAR_FLOAT, "Scale interpolationg rate when we fall behind");
idCVar net_interpolationBaseRate("net_interpolationBaseRate", "1.0", CVAR_FLOAT, "Scale interpolationg rate when we fall behind");

// Extrapolation is now disabled
idCVar net_maxExtrapolationInMS("net_maxExtrapolationInMS", "0", CVAR_INTEGER, "Max time in MS that extrapolation is allowed to occur.");

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
	if (net_debug_snapShot.GetBool()) {
		idLib::Printf("ProcessSnapshot time: %d\n", ss.GetTime());
	}

	int time = Sys_Milliseconds();

	snapTime = time;
	snapPrevious = snapCurrent;
	snapCurrent.serverTime = ss.GetTime();
	snapRate = snapCurrent.serverTime - snapPrevious.serverTime;


	static int lastReceivedLocalTime = 0;
	int timeSinceLastSnap = (time - lastReceivedLocalTime);
	if (net_debug_snapShotTime.GetBool()) {
		idLib::Printf("^2ProcessSnapshot. delta serverTime: %d  delta localTime: %d \n",
			(snapCurrent.serverTime - snapPrevious.serverTime), timeSinceLastSnap);
	}
	lastReceivedLocalTime = time;

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
	int oldTime = Game()->GetServerGameTimeMs();
	Game()->SetServerGameTimeMs(snapCurrent.serverTime);

	Game()->ClientReadSnapshot(ss); //, &oldss );

	// Restore server game time
	Game()->SetServerGameTimeMs(oldTime);

	snapTimeDelta = ss.GetRecvTime() - oldss.GetRecvTime();
	oldss = ss;
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
idCommonLocal::CalcSnapTimeBuffered
Return the amount of game time left of buffered snapshots
totalBufferedTime - total amount of snapshot time (includng what we've already past in current interpolate)
totalRecvTime - total real time (sys_milliseconds) all of totalBufferedTime was received over
========================
*/
int idCommonLocal::CalcSnapTimeBuffered(int& totalBufferedTime, int& totalRecvTime) {

	totalBufferedTime = snapRate;
	totalRecvTime = snapTimeDelta;

	// oldSS = last ss we deserialized
	int lastBuffTime = oldss.GetTime();
	int lastRecvTime = oldss.GetRecvTime();

	// receivedSnaps[readSnapshotIndex % RECEIVE_SNAPSHOT_BUFFER_SIZE] = next buffered snapshot we haven't processed yet (might not exist)
	for (int i = readSnapshotIndex; i < writeSnapshotIndex; i++) {
		int buffTime = receivedSnaps[i % RECEIVE_SNAPSHOT_BUFFER_SIZE].GetTime();
		int recvTime = receivedSnaps[i % RECEIVE_SNAPSHOT_BUFFER_SIZE].GetRecvTime();

		totalBufferedTime += buffTime - lastBuffTime;
		totalRecvTime += recvTime - lastRecvTime;

		lastRecvTime = recvTime;
		lastBuffTime = buffTime;
	}

	totalRecvTime = std::max(1, totalRecvTime);
	totalRecvTime = static_cast<float>(initialBaseTicksPerSec) * static_cast<float>(totalRecvTime / 1000.0f); // convert realMS to gameMS

	// remove time we've already interpolated over
	int timeLeft = totalBufferedTime - std::min(snapRate, snapCurrentTime);

	//idLib::Printf( "CalcSnapTimeBuffered. timeLeft: %d totalRecvTime: %d, totalTimeBuffered: %d\n", timeLeft, totalRecvTime, totalBufferedTime );
	return timeLeft;
}

/*
========================
idCommonLocal::InterpolateSnapshot
========================
*/
void idCommonLocal::InterpolateSnapshot(netTimes_t& prev, netTimes_t& next, float fraction, bool predict) {

	int serverTime = Lerp(prev.serverTime, next.serverTime, fraction);

	Game()->SetServerGameTimeMs(serverTime);		// Set the global server time to the interpolated time of the server
	Game()->SetInterpolation(fraction, serverTime, prev.serverTime, next.serverTime);

	//Game()->RunFrame( &userCmdMgr, &ret, true );

}

/*
========================
idCommonLocal::RunNetworkSnapshotFrame
========================
*/
void idCommonLocal::RunNetworkSnapshotFrame() {
	// Process any reliable messages we've received
	/*for (int i = 0; i < reliableQueue.size(); i++) {
		game->ProcessReliableMessage(reliableQueue[i].client, reliableQueue[i].type,
			idBitMsg(reliableQueue[i].data, reliableQueue[i].dataSize));
		Mem_Free(reliableQueue[i].data);
	}
	reliableQueue.clear();*/

	// abuse the game timing to time presentable thinking on clients
	time_gameFrame = Sys_Microseconds();
	time_maxGameFrame = 0;
	count_numGameFrames = 0;

	if (snapPrevious.serverTime >= 0) {

		int	msec_interval = 1 + static_cast<int>(initialBaseTicksPerSec);

		static int clientTimeResidual = 0;
		static int lastTime = Sys_Milliseconds();
		int currentTime = Sys_Milliseconds();
		int deltaFrameTime = idMath::ClampInt(1, 33, currentTime - lastTime);

		clientTimeResidual += idMath::ClampInt(0, 50, currentTime - lastTime);
		lastTime = currentTime;

		extern idCVar com_fixedTic;
		if (com_fixedTic.GetBool()) {
			clientTimeResidual = 0;
		}

		do {
			// If we are extrapolating and have fresher snapshots, then use the freshest one
			while ((snapCurrentTime >= snapRate || com_forceLatestSnap.GetBool()) && readSnapshotIndex < writeSnapshotIndex) {
				snapCurrentTime -= snapRate;
				ProcessNextSnapshot();
			}

			// Clamp the current time so that it doesn't fall outside of our extrapolation bounds
			//snapCurrentTime = idMath::ClampInt(0, snapRate +
			//	std::min(static_cast<int>(snapRate), net_maxExtrapolationInMS.GetInteger()), snapCurrentTime);

			//if (snapRate <= 0) {
			//	idLib::Warning("snapRate <= 0. Resetting to 100");
			//	snapRate = 100;
			//}

			//float fraction = (float)snapCurrentTime / (float)snapRate;
			//if (!IsValid(fraction)) {
			//	idLib::Warning("Interpolation Fraction invalid: snapCurrentTime %d / snapRate %d",
			//		(int)snapCurrentTime, (int)snapRate);
			//	fraction = 0.0f;
			//}

			//InterpolateSnapshot(snapPrevious, snapCurrent, fraction, true);

			//// Default to a snap scale of 1
			//float snapRateScale = net_interpolationBaseRate.GetFloat();

			//snapTimeBuffered = CalcSnapTimeBuffered(totalBufferedTime, totalRecvTime);
			//effectiveSnapRate = static_cast<float> (totalBufferedTime) / static_cast<float> (totalRecvTime);

			//if (net_minBufferedSnapPCT_Static.GetFloat() > 0.0f) {
			//	optimalPCTBuffer = session->GetTitleStorageFloat("net_minBufferedSnapPCT_Static", net_minBufferedSnapPCT_Static.GetFloat());
			//}

			//// Calculate optimal amount of buffered time we want
			//if (net_optimalDynamic.GetBool()) {
			//	optimalTimeBuffered = idMath::ClampInt(0, net_maxBufferedSnapMS.GetInteger(), snapRate * optimalPCTBuffer);
			//	optimalTimeBufferedWindow = snapRate * net_minBufferedSnapWinPCT_Static.GetFloat();
			//}
			//else {
			//	optimalTimeBuffered = net_optimalSnapTime.GetFloat();
			//	optimalTimeBufferedWindow = net_optimalSnapWindow.GetFloat();
			//}

			//// Scale snapRate based on where we are in the buffer
			//if (snapTimeBuffered <= optimalTimeBuffered) {
			//	if (snapTimeBuffered <= idMath::FLT_SMALLEST_NON_DENORMAL) {
			//		snapRateScale = 0;
			//	}
			//	else {
			//		snapRateScale = net_interpolationFallbackRate.GetFloat();
			//		// When we interpolate past our cushion of buffered snapshot, we want to slow smoothly slow the
			//		// rate of interpolation. frac will go from 1.0 to 0.0 (if snapshots stop coming in).
			//		float startSlowdown = (net_interpolationSlowdownStart.GetFloat() * optimalTimeBuffered);
			//		if (startSlowdown > 0 && snapTimeBuffered < startSlowdown) {
			//			float frac = idMath::ClampFloat(0.0f, 1.0f, snapTimeBuffered / startSlowdown);
			//			if (!IsValid(frac)) {
			//				frac = 0.0f;
			//			}
			//			snapRateScale = Square(frac) * snapRateScale;
			//			if (!IsValid(snapRateScale)) {
			//				snapRateScale = 0.0f;
			//			}
			//		}
			//	}


			//}
			//else if (snapTimeBuffered > optimalTimeBuffered + optimalTimeBufferedWindow) {
			//	// Go faster
			//	snapRateScale = net_interpolationCatchupRate.GetFloat();

			//}

			//float delta_interpolate = (float)initialBaseTicksPerSec * snapRateScale;
			//if (net_effectiveSnapRateEnable.GetBool()) {

			//	float deltaFrameGameMS = static_cast<float>(initialBaseTicksPerSec) * static_cast<float>(deltaFrameTime / 1000.0f);
			//	delta_interpolate = (deltaFrameGameMS * snapRateScale * effectiveSnapRate) + snapCurrentResidual;
			//	if (!IsValid(delta_interpolate)) {
			//		delta_interpolate = 0.0f;
			//	}

			//	snapCurrentResidual = idMath::Frac(delta_interpolate); // fixme: snapCurrentTime should just be a float, but would require changes in d4 too
			//	if (!IsValid(snapCurrentResidual)) {
			//		snapCurrentResidual = 0.0f;
			//	}

			//	if (net_effectiveSnapRateDebug.GetBool()) {
			//		idLib::Printf("%d/%.2f snapRateScale: %.2f effectiveSR: %.2f d.interp: %.2f snapTimeBuffered: %.2f res: %.2f\n", deltaFrameTime, deltaFrameGameMS, snapRateScale, effectiveSnapRate, delta_interpolate, snapTimeBuffered, snapCurrentResidual);
			//	}
			//}

			//assert(IsValid(delta_interpolate));
			//int interpolate_interval = idMath::Ftoi(delta_interpolate);

			//snapCurrentTime += interpolate_interval;	// advance interpolation time by the scaled interpolate_interval
			clientTimeResidual -= msec_interval;		// advance local client residual time (fixed step)

		} while (clientTimeResidual >= msec_interval);

		if (clientTimeResidual < 0) {
			clientTimeResidual = 0;
		}
	}

	time_gameFrame = Sys_Microseconds() - time_gameFrame;
}

/*
========================
idCommonLocal::ResetNetworkingState
========================
*/
void idCommonLocal::ResetNetworkingState() {
	snapTime = 0;
	snapCurrentTime = 0;
	//snapCurrentResidual = 0.0f;

	snapTimeBuffered = 0.0f;
	effectiveSnapRate = 0.0f;
	totalBufferedTime = 0;
	totalRecvTime = 0;

	readSnapshotIndex = 0;
	writeSnapshotIndex = 0;
	snapRate = 100000;
	/*optimalTimeBuffered = 0.0f;
	optimalPCTBuffer = 0.5f;
	optimalTimeBufferedWindow = 0.0;*/

	// Clear snapshot queue
	for (int i = 0; i < RECEIVE_SNAPSHOT_BUFFER_SIZE; i++) {
		receivedSnaps[i].Clear();
	}

	userCmdMgr.SetDefaults();

	snapCurrent.localTime = -1;
	snapPrevious.localTime = -1;
	snapCurrent.serverTime = -1;
	snapPrevious.serverTime = -1;

	// Make sure our current snap state is cleared so state from last game doesn't carry over into new game
	oldss.Clear();

	gameFrame = 0;
	/*clientPrediction = 0;
	nextUsercmdSendTime = 0;*/
	nextSnapshotSendTime = 0;
}