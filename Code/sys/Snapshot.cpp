#include "idlib/precompiled.h"

idCVar net_verboseSnapshot("net_verboseSnapshot", "0", CVAR_INTEGER | CVAR_NOCHEAT, "Verbose snapshot code to help debug snapshot problems. Greater the number greater the spam");

/*
========================
idSnapShot::idSnapShot
========================
*/
idSnapShot::idSnapShot() :
	time(0),
	recvTime(0)
{
}

/*
========================
idSnapShot::~idSnapShot
========================
*/
idSnapShot::~idSnapShot() {
	Clear();
}

/*
========================
idSnapShot::Clear
========================
*/
void idSnapShot::Clear() {
	time = 0;
	recvTime = 0;
}

/*
========================
idSnapShot::ReadDeltaForJob
========================
*/
bool idSnapShot::ReadDeltaForJob(const std::byte* deltaMem, int deltaSize, int& outSeq, int& outBaseSeq) {
	// Skip past sequence and baseSequence
	int sequence = 0;
	int baseSequence = 0;

	int objectNum = 0;

	idBitMsg msg(deltaMem, deltaSize);

	sequence = msg.ReadLong();
	baseSequence = msg.ReadLong();
	time = msg.ReadLong();

	outSeq = sequence;
	outBaseSeq = baseSequence;

	int num_objects = msg.ReadLong();

	for (size_t i = 0; i < num_objects; ++i) {
		objectState_t state;
		state.objectNum = objectNum++;

		int size = msg.ReadLong();

		idassert(size >= 0 && size < 1024);

		// the buffer shrank or stayed the same
		objectBuffer_t newbuffer(size);

		state.buffer = std::move(newbuffer);
		msg.ReadData(state.buffer.Ptr(), size);

		allocatedObjs.push_back(std::move(state));
	}
	
	return true;
}

/*
========================
idSnapShot::AddObject
========================
*/
idSnapShot::objectState_t* idSnapShot::S_AddObject(int objectNum, const std::byte* data, int _size) {
	uint32_t size = _size;
	objectBuffer_t buffer(size);
	memcpy(buffer.Ptr(), data, size);

	objectState_t object_state;
	object_state.objectNum = objectNum;
	object_state.buffer = std::move(buffer);

	allocatedObjs.push_back(std::move(object_state));

	return nullptr;
}

/*
========================
idSnapShot::GetObjectMsgByIndex
========================
*/
int idSnapShot::GetObjectMsgByIndex(int i, idBitMsg& msg, bool ignoreIfStale) const {
	if (i < 0 || i >= allocatedObjs.size()) {
		return -1;
	}

	const auto& obj = allocatedObjs[i];

	msg.InitRead(obj.buffer.Ptr(), obj.buffer.Size());
	return obj.objectNum;
}