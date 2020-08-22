#include "sys_signin.h"

/*
========================
idSignInManagerBase::GetDefaultProfile
========================
*/
idPlayerProfile* idSignInManagerBase::GetDefaultProfile() {
	if (!defaultProfile) {
		// Create a new profile
		defaultProfile = idPlayerProfile::CreatePlayerProfile(0);
	}
	return defaultProfile;
}