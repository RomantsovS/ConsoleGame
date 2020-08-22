#ifndef SYS_WIN32_WIN_LOCALUSER_H_
#define SYS_WIN32_WIN_LOCALUSER_H_

#include "../sys_localuser.h"

/*
================================================
idLocalUserWin
================================================
*/
class idLocalUserWin : public idLocalUser {
public:
	/*static const int MAX_GAMERTAG = 64;			// max number of bytes for a gamertag
	static const int MAX_GAMERTAG_CHARS = 16;	// max number of UTF-8 characters to show
	*/
	idLocalUserWin() : inputDevice(0) {}

	//==========================================================================================
	// idLocalUser interface
	//==========================================================================================
	virtual int					GetInputDevice() const { return inputDevice; }

	//==========================================================================================
	// idLocalUserWin interface
	//==========================================================================================

private:
	int inputDevice;
};

#endif
