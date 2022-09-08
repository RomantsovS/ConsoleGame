
#ifndef __SNAP_PROCESSOR_H__
#define __SNAP_PROCESSOR_H__

/*
================================================
idSnapshotProcessor
================================================
*/
class idSnapshotProcessor {
public:
	static const int INITIAL_SNAP_SEQUENCE = 42;

	idSnapshotProcessor();
	~idSnapshotProcessor();

	void Reset(bool cstor = false);

	// TrySetPendingSnapshot Sets the currently pending snap.  
	// No new snaps will be sent until this snap has been fully sent.
	// Returns true of the newly supplied snapshot was accepted (there were no pending snaps)
	bool TrySetPendingSnapshot(idSnapShot& ss);
	// Attempts to write the currently pending snap to the supplied buffer, which can then be sent as an unreliable msg.
	// SubmitPendingSnap will submit the pending snap to a job, so that it can be retrieved later for sending.
	void SubmitPendingSnap();
	// GetPendingSnapDelta
	int GetPendingSnapDelta(std::byte* outBuffer, int maxLength);

	// HasPendingSnap will return true if there is more of the last TrySetPendingSnapshot to be sent
	bool HasPendingSnap() const { return hasPendingSnap; }

	static const int MAX_SNAPSHOT_QUEUE = 64;
private:
	static const int MAX_SNAPSHOT_QUEUE_MEM = 64 * 1024;	// 64k

	idDataQueue< MAX_SNAPSHOT_QUEUE, MAX_SNAPSHOT_QUEUE_MEM > deltas;		// list of unacknowledged snapshot deltas

	int reliableSequenceSend; // sequence number of the next reliable packet we're going to send to this peer
	int reliableSequenceRecv; // sequence number of the last reliable packet we received from this peer

	idSnapShot pendingSnap;		// Current snap waiting to be fully sent
	bool hasPendingSnap;		// true if pendingSnap is still waiting to be sent
};

#endif