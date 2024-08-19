#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

extern idCVar net_verboseSnapshot;
#define NET_VERBOSESNAPSHOT_PRINT	if ( net_verboseSnapshot.GetInteger() > 0 ) idLib::Printf
#define NET_VERBOSESNAPSHOT_PRINT_LEVEL( X, Y )  if ( net_verboseSnapshot.GetInteger() >= ( X ) ) idLib::Printf( Y )

/*
A snapshot contains a list of objects and their states
*/
class idSnapShot {
public:
	idSnapShot();
	//idSnapShot(const idSnapShot& other);
	~idSnapShot();

	//void operator=(const idSnapShot& other);

	// clears the snapshot
	void Clear();

	int  GetTime() const { return time; }
	void SetTime(int t) { time = t; }

	int  GetRecvTime() const { return recvTime; }
	void SetRecvTime(int t) { recvTime = t; }

	// Reads a new object state packet, which is assumed to be delta compressed against this snapshot
	bool ReadDeltaForJob(const std::byte* deltaMem, int deltaSize, int& outSeq, int& outBaseSeq);

	// Writes an object state packet which is delta compressed against the old snapshot
	struct objectBuffer_t {
		objectBuffer_t() = default;
		objectBuffer_t(int s) { data.resize(s); }

		uint32_t Size() const { return data.size(); }
		std::byte* Ptr() { return data.empty() ? nullptr : data.data(); }
		const std::byte* Ptr() const { return data.empty() ? nullptr : data.data(); }
		std::byte& operator[](int i) { return data[i]; }
	private:
		std::vector<std::byte> data;
	};

	struct objectState_t {
		objectState_t() : objectNum(0) { }

		uint16_t objectNum;
		objectBuffer_t buffer;
	};

	// Adds an object to the state, overwrites any existing object with the same number
	objectState_t* S_AddObject(int objectNum, const idBitMsg& msg) { return S_AddObject(objectNum, msg.GetReadData(), msg.GetSize()); }
	objectState_t* S_AddObject(int objectNum, const std::byte* buffer, int size);

	// returns the number of objects in this snapshot
	int NumObjects() const { return allocatedObjs.size(); }

	// Returns the object number of the specified object, also fills the bitmsg
	int GetObjectMsgByIndex(int i, idBitMsg& msg, bool ignoreIfStale = false) const;
private:
	std::vector<objectState_t> allocatedObjs;

	int time;
	int recvTime;
};

#endif