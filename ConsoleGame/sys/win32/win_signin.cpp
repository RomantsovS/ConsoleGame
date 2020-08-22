#include "win_signin.h"

/*
========================
idSignInManagerWin::Shutdown
========================
*/
void idSignInManagerWin::Shutdown() {
}

/*
========================
idSignInManagerWin::Pump
========================
*/
void idSignInManagerWin::Pump() {

	// If we have more users than we need, then set to the lower amount
	// (don't remove the master user though)
	/*if (localUsers.Num() > 1 && localUsers.Num() > maxDesiredLocalUsers) {
		localUsers.SetNum(maxDesiredLocalUsers);
	}*/

#ifndef ID_RETAIL
	// If we don't have enough, then make sure we do
	// NOTE - We always want at least one user on windows for now, 
	// and this master user will always use controller 0
	/*while (localUsers.Num() < minDesiredLocalUsers) {
		RegisterLocalUser(localUsers.Num());
	}*/
#endif

	// See if we need to save settings on any of the profiles
	for (size_t i = 0; i < localUsers.size(); i++) {
		localUsers[i].Pump();
	}
}