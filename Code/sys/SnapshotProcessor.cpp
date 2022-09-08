#include "idlib/precompiled.h"

/*
========================
idSnapshotProcessor::idSnapshotProcessor
========================
*/
idSnapshotProcessor::idSnapshotProcessor() {
	Reset(true);
}

/*
========================
idSnapshotProcessor::idSnapshotProcessor
========================
*/
idSnapshotProcessor::~idSnapshotProcessor() {
}

/*
========================
idSnapshotProcessor::Reset
========================
*/
void idSnapshotProcessor::Reset(bool cstor) {
	hasPendingSnap = false;
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
idSnapshotProcessor::SubmitPendingSnap
========================
*/
void idSnapshotProcessor::SubmitPendingSnap() {
	deltas.Append(reliableSequenceSend++, pendingSnap.GetData(), pendingSnap.GetSize());
}

/*
========================
idSnapshotProcessor::GetPendingSnapDelta
========================
*/
int idSnapshotProcessor::GetPendingSnapDelta(std::byte* outBuffer, int maxLength) {

	if (deltas.Num() > 0) {
		// Copy to out buffer
		memcpy(outBuffer, deltas.ItemData(0), deltas.ItemLength(0));
	}

	return 1;
}