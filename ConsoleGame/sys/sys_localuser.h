#ifndef SYS_SYS_LOCALUSER_H_
#define SYS_SYS_LOCALUSER_H_

#include "sys_profile.h"

/*
================================================
idLocalUser
An idLocalUser is a user holding a controller.
It represents someone controlling the menu or game.
They may not necessarily be in a game (which would be a session user of TYPE_GAME).
A controller user references an input device (which is a gamepad, keyboard, etc).
================================================
*/
class idLocalUser {
public:
	idLocalUser();
	virtual ~idLocalUser() {}

	void Pump();

	virtual int GetInputDevice() const = 0; // Input device of controller

	idProfileMgr& GetProfileMgr() { return profileMgr; }

private:
	idProfileMgr profileMgr;
};

#endif