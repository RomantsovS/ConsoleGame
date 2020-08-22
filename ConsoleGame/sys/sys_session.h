#ifndef SYS_SYS_SESSION_H_ 
#define SYS_SYS_SESSION_H_

#include <memory>
#include "sys_signin.h"

/*
================================================
idSession
================================================
*/
class idSession {
public:

	enum sessionState_t {
		PRESS_START,
		IDLE,
		SEARCHING,
		CONNECTING,
		PARTY_LOBBY,
		GAME_LOBBY,
		LOADING,
		INGAME,
		BUSY,
		MAX_STATES
	};

	idSession() :
		signInManager(nullptr) {}

	virtual 		~idSession();

	virtual void			Initialize() = 0;
	virtual void			Shutdown() = 0;

	//=====================================================================================================
	//	GamerCard UI
	//=====================================================================================================

	virtual void				UpdateSignInManager() = 0;

protected:
	std::shared_ptr<idSignInManagerBase> signInManager; // pointer so we can treat dynamically bind platform-specific impl

};

#endif