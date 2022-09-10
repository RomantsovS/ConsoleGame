#include "precompiled.h"

/*
========================
idBitMsg::CheckOverflow
========================
*/
bool idBitMsg::CheckOverflow(int numBytes) {
	if (curSize + numBytes > GetRemainingWriteBytes()) {
		idLib::FatalError("idBitMsg: overflow without allowOverflow set; maxsize=%i size=%i numBytes=%i numRemainingWriteBytes=%i",
		GetMaxSize(), GetSize(), numBytes, GetRemainingWriteBytes());
		return true;
	}
	return false;
}

/*
========================
idBitMsg::GetByteSpace
========================
*/
std::byte* idBitMsg::GetByteSpace(int length) {
	if (!writeData) {
		idLib::FatalError("idBitMsg::GetByteSpace: cannot write to message");
	}

	// check for overflow
	CheckOverflow(length);

	std::byte* ptr = writeData + curSize;
	curSize += length;
	return ptr;
}

void idBitMsg::WriteBytes(int32_t value, int numBytes) {
	if (!writeData) {
		idLib::FatalError("idBitMsg::WriteBits: cannot write to message");
	}

	// check for msg overflow
	if (CheckOverflow(numBytes)) {
		return;
	}

	std::byte* bytes = reinterpret_cast<std::byte*>(&value);
	for (int i = 0; i < numBytes; ++i) {
		writeData[curSize + i] = bytes[numBytes - i - 1];
	}
	curSize += numBytes;
}

/*
========================
idBitMsg::ReadBits

If the number of bits is negative a sign is included.
========================
*/
int idBitMsg::ReadBytes(int numBytes) const {
	int value = 0;
	int fraction;

	if (!readData) {
		idLib::FatalError("idBitMsg::ReadBits: cannot read from message");
	}

	for (size_t i = 0; i < numBytes; ++i) {
		fraction = static_cast<int>(readData[readCount + i]);
		value |= fraction << (numBytes - i - 1) * 8;
	}
	readCount += numBytes;

	return value;
}

/*
========================
idBitMsg::WriteData
========================
*/
void idBitMsg::WriteData(const void* data, int length) {
	memcpy(GetByteSpace(length), data, length);
}

bool idBitMsg::ReadProtobufMessage(google::protobuf::Message* proto_msg) const {
	uint64_t size = ReadLongLong();

	return proto_msg->ParseFromArray(readData + readCount, size);
}

bool idBitMsg::WriteProtobufMessage(google::protobuf::Message* proto_msg) {
	size_t size = proto_msg->ByteSizeLong();
	
	// check for msg overflow
	if (CheckOverflow(size)) {
		return false;
	}

	WriteLongLong(size);

	bool res = proto_msg->SerializeToArray(static_cast<void*>(writeData + curSize), size);
	curSize += size;

	return res;
}