
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
  // Returns true of the newly supplied snapshot was accepted (there were no
  // pending snaps)
  bool TrySetPendingSnapshot(idSnapShot& ss);
  // Apply a delta to the supplied snapshot
  bool ApplyDeltaToSnapshot(idSnapShot& snap, int& outSeq, int& outBaseSeq,
                            const std::byte* deltaMem, int deltaSize);
  // Attempts to write the currently pending snap to the supplied buffer, which
  // can then be sent as an unreliable msg. SubmitPendingSnap will submit the
  // pending snap to a job, so that it can be retrieved later for sending.
  void SubmitPendingSnap(std::byte* objMemory, size_t objMemorySize);
  // GetPendingSnapDelta
  int GetPendingSnapDelta(std::byte* outBuffer, int maxLength);
  // If PendingSnapReadyToSend is true, then GetPendingSnapDelta will return
  // something to send
  bool PendingSnapReadyToSend() const { return job_memory_size > 0; }
  // When you call WritePendingSnapshot, and then send the resulting buffer as a
  // unreliable msg, you will eventually receive this on the client.  Call this
  // function to receive and apply it to the base state, and possibly return a
  // fully received snap to then apply to the client game state
  bool ReceiveSnapshotDelta(const std::byte* deltaData, int deltaLength,
                            int& outSeq, int& outBaseSeq, idSnapShot& outSnap,
                            bool& fullSnap);

  // HasPendingSnap will return true if there is more of the last
  // TrySetPendingSnapshot to be sent
  bool HasPendingSnap() const { return hasPendingSnap; }

  int GetSnapSequence() { return snapSequence; }
  int GetBaseSequence() { return baseSequence; }

  static const int MAX_SNAPSHOT_QUEUE = 64;
  static const int MAX_SNAPSHOT_QUEUE_MEM = 64 * 1024;  // 64k
 private:
  std::array<std::byte, MAX_SNAPSHOT_QUEUE_MEM> jobMemory;
  size_t job_memory_size = 0;

  // sequence number of the last snapshot we sent/received
  // on the server, the sequencing is different for each network peer
  // (net_verboseSnapshot 1) on the jobbed snapshot compression path, the
  // sequence is incremented in NewLZWStream and pulled into this in
  // idSnapshotProcessor::GetPendingSnapDelta
  int snapSequence;
  int baseSequence;

  idSnapShot baseState;  // known snapshot base on the client
  idDataQueue<MAX_SNAPSHOT_QUEUE, MAX_SNAPSHOT_QUEUE_MEM>
      deltas;  // list of unacknowledged snapshot deltas

  idSnapShot pendingSnap;  // Current snap waiting to be fully sent
  bool hasPendingSnap;     // true if pendingSnap is still waiting to be sent
};

#endif