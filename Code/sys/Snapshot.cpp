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