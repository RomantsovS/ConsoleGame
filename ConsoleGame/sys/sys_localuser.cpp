#include "sys_localuser.h"

/*
========================
idLocalUser::idLocalUser
========================
*/
idLocalUser::idLocalUser() {
	profileMgr.Init(this);
}

void idLocalUser::Pump() {
	// Pump the profile
	GetProfileMgr().Pump();
}