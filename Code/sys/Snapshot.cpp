#include "idlib/precompiled.h"

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