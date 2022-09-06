#ifndef __BITMSG_H__
#define __BITMSG_H__

#include <google/protobuf/util/delimited_message_util.h>
#include <google/protobuf/message.h>

class idBitMsg {
public:
	idBitMsg() { InitWrite(nullptr, 0); }
	idBitMsg(std::byte* data, int length) { InitWrite(data, length); }
	idBitMsg(const std::byte* data, int length) { InitRead(data, length); }

	// both read & write
	void InitWrite(std::byte* data, int length);

	// read only
	void InitRead(const std::byte* data, int length);

	// get data for writing
	std::byte* GetWriteData();

	// get data for reading
	const std::byte* GetReadData() const;

	// get the maximum message size
	int GetMaxSize() const;

	// size of the message in bytes
	int GetSize() const;

	// set the message size
	void SetSize(int size);

	// returns number of bits written
	int GetNumBitsWritten() const;

	// space left in bits for writing
	int GetRemainingWriteBits() const;

	//------------------------
	// Write State
	//------------------------
	// 

	// save the read state
	void SaveReadState(int& c, int& b) const;

	// restore the read state
	void RestoreReadState(int c, int b);

	//------------------------
	// Reading
	//------------------------

	// bytes read so far
	int GetReadCount() const;

	// number of bytes left to read
	int GetRemainingData() const;

	// read the specified number of bits
	int ReadBytes(int numBytes) const;

	uint32_t ReadLong();
	uint64_t ReadLongLong();
	bool ReadProtobufMessage(google::protobuf::Message* proto_msg);

	//------------------------
	// Writing
	//------------------------

	// begin writing
	void BeginWriting();

	// write the specified number of bytes
	void WriteBytes(int32_t value, int numBytes);

	void WriteUShort(uint16_t c);
	void WriteLong(int32_t c);
	void WriteLongLong(int64_t c);
	void WriteData(const void* data, int length);
	bool WriteProtobufMessage(google::protobuf::Message* proto_msg);
private:
	std::byte* writeData;		// pointer to data for writing
	const std::byte* readData;		// pointer to data for reading
	int maxSize;		// maximum size of message in bytes
	int curSize;		// current size of message in bytes
	mutable int writeBit;		// number of bits written to the last written byte
	mutable int readCount;		// number of bytes read so far

	mutable uint64_t tempValue;
private:
	bool CheckOverflow(int numBits);
	std::byte* GetByteSpace(int length);
};

/*
========================
idBitMsg::InitWrite
========================
*/
inline void idBitMsg::InitWrite(std::byte* data, int length) {
	writeData = data;
	readData = data;
	maxSize = length;
	curSize = 0;

	writeBit = 0;
	readCount = 0;

	tempValue = 0;
}

/*
========================
idBitMsg::InitRead
========================
*/
inline void idBitMsg::InitRead(const std::byte* data, int length) {
	writeData = nullptr;
	readData = data;
	maxSize = length;
	curSize = length;

	writeBit = 0;
	readCount = 0;

	tempValue = 0;
}

/*
========================
idBitMsg::GetWriteData
========================
*/
inline std::byte* idBitMsg::GetWriteData() {
	return writeData;
}

/*
========================
idBitMsg::GetReadData
========================
*/
inline const std::byte* idBitMsg::GetReadData() const {
	return readData;
}

/*
========================
idBitMsg::GetMaxSize
========================
*/
inline int idBitMsg::GetMaxSize() const {
	return maxSize;
}

/*
========================
idBitMsg::GetSize
========================
*/
inline int idBitMsg::GetSize() const {
	return curSize + (writeBit != 0);
}

/*
========================
idBitMsg::SetSize
========================
*/
inline void idBitMsg::SetSize(int size) {
	assert(writeBit == 0);

	if (size > maxSize) {
		curSize = maxSize;
	}
	else {
		curSize = size;
	}
}

/*
========================
idBitMsg::GetNumBitsWritten
========================
*/
inline int idBitMsg::GetNumBitsWritten() const {
	return (curSize << 3) + writeBit;
}

/*
========================
idBitMsg::GetRemainingWriteBits
========================
*/
inline  int idBitMsg::GetRemainingWriteBits() const {
	return (maxSize << 3) - GetNumBitsWritten();
}

/*
========================
idBitMsg::GetReadCount
========================
*/
inline int idBitMsg::GetReadCount() const {
	return readCount;
}

/*
========================
idBitMsg::GetRemainingData
========================
*/
inline int idBitMsg::GetRemainingData() const {
	assert(writeBit == 0);
	return curSize - readCount;
}

/*
========================
idBitMsg::SaveReadState
========================
*/
inline void idBitMsg::SaveReadState(int& c, int& b) const {
	//c = input->CurrentPosition();
}

/*
========================
idBitMsg::RestoreReadState
========================
*/
inline void idBitMsg::RestoreReadState(int c, int b) {
	//input->
}

/*
========================
idBitMsg::BeginWriting
========================
*/
inline void idBitMsg::BeginWriting() {
	curSize = 0;

	writeBit = 0;
	tempValue = 0;
}

/*
========================
idBitMsg::WriteUShort
========================
*/
inline void idBitMsg::WriteUShort(uint16_t c) {
	WriteBytes(c, 2);
}

/*
========================
idBitMsg::WriteLong
========================
*/
inline void idBitMsg::WriteLong(int32_t c) {
	WriteBytes(c, 4);
}

/*
========================
idBitMsg::WriteLongLong
========================
*/
inline void idBitMsg::WriteLongLong(int64_t c) {
	int a = c;
	int b = c >> 32;
	WriteBytes(b, 4);
	WriteBytes(a, 4);
}

inline uint32_t idBitMsg::ReadLong() {
	return ReadBytes(4);
}

inline uint64_t idBitMsg::ReadLongLong() {
	int64 a = ReadBytes(4);
	int64 b = ReadBytes(4);
	int64 c = (a << 32) | (0x00000000ffffffff & b);
	return c;
}

#endif