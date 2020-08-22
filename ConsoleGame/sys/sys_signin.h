#ifndef SYS_SYS_SIGNIN_H_
#define SYS_SYS_SIGNIN_H_

#include <memory>
#include "../framework/PlayerProfile.h"

/*
================================================
idSignInManagerBase
================================================
*/
class idSignInManagerBase {
public:

	idSignInManagerBase() :
		defaultProfile(nullptr) {}
	
	virtual ~idSignInManagerBase() {}

	virtual void					Pump() = 0;
	virtual void					Shutdown() {}

	//================================================================================
	// Common helper functions
	//================================================================================
	idPlayerProfile* GetDefaultProfile();
protected:
	idPlayerProfile* defaultProfile;
};

#endif