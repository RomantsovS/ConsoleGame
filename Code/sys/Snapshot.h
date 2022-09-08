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

	void Add(const idBitMsg& msg) {
		memcpy(buffer.data(), msg.GetReadData(), msg.GetSize());
		size = msg.GetSize();
	}
	std::byte* GetData() { return buffer.data(); }
	const std::byte* GetData() const { return buffer.data(); }
	size_t GetSize() const { return size; }
private:

	int time;
	int recvTime;

	std::array<std::byte, 4096> buffer;
	size_t size = 0;
};

#endif