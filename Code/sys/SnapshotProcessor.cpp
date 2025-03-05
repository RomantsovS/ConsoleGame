#include "idlib/precompiled.h"

/*
========================
idSnapshotProcessor::idSnapshotProcessor
========================
*/
idSnapshotProcessor::idSnapshotProcessor() { Reset(true); }

/*
========================
idSnapshotProcessor::idSnapshotProcessor
========================
*/
idSnapshotProcessor::~idSnapshotProcessor() {}

/*
========================
idSnapshotProcessor::Reset
========================
*/
void idSnapshotProcessor::Reset(bool cstor) {
  hasPendingSnap = false;
  snapSequence = INITIAL_SNAP_SEQUENCE;
  baseSequence = -1;

  baseState.Clear();
  pendingSnap.Clear();
  deltas.Clear();
}

/*
========================
idSnapshotProcessor::TrySetPendingSnapshot
========================
*/
bool idSnapshotProcessor::TrySetPendingSnapshot(idSnapShot& ss) {
  // Don't advance to the next snap until the last one was fully sent
  if (hasPendingSnap) {
    return false;
  }
  pendingSnap = ss;
  hasPendingSnap = true;
  return true;
}

/*
========================
idSnapshotProcessor::ApplyDeltaToSnapshot
========================
*/
bool idSnapshotProcessor::ApplyDeltaToSnapshot(idSnapShot& snap, int& outSeq,
                                               int& outBaseSeq,
                                               const std::byte* deltaMem,
                                               int deltaSize) {
  return snap.ReadDeltaForJob(deltaMem, deltaSize, outSeq, outBaseSeq);
}

/*
========================
idSnapshotProcessor::SubmitPendingSnap
========================
*/
void idSnapshotProcessor::SubmitPendingSnap(std::byte* objMemory,
                                            size_t objMemorySize) {
  idassert(hasPendingSnap);

  idBitMsg out_msg;
  out_msg.InitWrite(jobMemory.data(), MAX_SNAPSHOT_QUEUE_MEM);

  out_msg.WriteLong(snapSequence);
  out_msg.WriteLong(baseSequence);
  out_msg.WriteLong(pendingSnap.GetTime());

  out_msg.WriteLong(pendingSnap.NumObjects());

  for (int o = 0; o < pendingSnap.NumObjects(); o++) {
    idBitMsg msg;
    int snapObjectNum = pendingSnap.GetObjectMsgByIndex(o, msg);
    out_msg.WriteLong(msg.GetSize());
    out_msg.WriteData(msg.GetReadData(), msg.GetSize());
  }

  job_memory_size = out_msg.GetSize();

  snapSequence++;
}

/*
========================
idSnapshotProcessor::GetPendingSnapDelta
========================
*/
int idSnapshotProcessor::GetPendingSnapDelta(std::byte* outBuffer,
                                             int maxLength) {
  ididassert(PendingSnapReadyToSend());

  ididassert(hasPendingSnap);

  size_t size = job_memory_size;

  // Copy to out buffer
  memcpy(outBuffer, jobMemory.data(), size);

  job_memory_size = 0;

  // deltas.Append(snapSequence++, jobMemory.data(), job_memory_size);
  hasPendingSnap = false;

  return size;
}

/*
========================
idSnapshotProcessor::ReceiveSnapshotDelta
NOTE: we use ReadDeltaForJob twice, once to build the same base as the server
(based on server acks, down ApplySnapshotDelta), and another time to apply the
snapshot we just received could we avoid the double apply by keeping outSnap
cached in memory and avoid rebuilding it from a delta when the next one comes
around?
========================
*/
bool idSnapshotProcessor::ReceiveSnapshotDelta(const std::byte* deltaData,
                                               int deltaLength, int& outSeq,
                                               int& outBaseSeq,
                                               idSnapShot& outSnap,
                                               bool& fullSnap) {
  fullSnap = false;

  // Make a copy of the basestate the server used to create this delta, and then
  // apply and return it
  outSnap = baseState;

  fullSnap =
      ApplyDeltaToSnapshot(outSnap, outSeq, outBaseSeq, deltaData, deltaLength);

  // We received a new delta
  return true;
}