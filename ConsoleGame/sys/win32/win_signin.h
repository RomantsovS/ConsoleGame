#ifndef SYS_WIN32_WIN_SIGNIN_H_
#define SYS_WIN32_WIN_SIGNIN_H_

#include <vector>
#include "../sys_signin.h"
#include "win_localuser.h"

/*
================================================
idSignInManagerWin
================================================
*/
class idSignInManagerWin : public idSignInManagerBase {
public:

	idSignInManagerWin() {}
	virtual ~idSignInManagerWin() {}

	//==========================================================================================
	// idSignInManagerBase interface
	//==========================================================================================
	virtual void					Pump() override;
	virtual void					Shutdown() override;
	/*virtual int						GetNumLocalUsers() const { return localUsers.Num(); }
	virtual idLocalUser* GetLocalUserByIndex(int index) { return &localUsers[index]; }
	virtual const idLocalUser* GetLocalUserByIndex(int index) const { return &localUsers[index]; }
	virtual void					RemoveLocalUserByIndex(int index);
	virtual void					RegisterLocalUser(int inputDevice);		// Register a local user to the passed in controller

	bool							CreateNewUser(winUserState_t& state);*/

private:
	std::vector<idLocalUserWin>	localUsers;
	bool dlcVersionChecked;
};

#endif
