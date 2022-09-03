#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

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
private:

	int time;
	int recvTime;
};

#endif