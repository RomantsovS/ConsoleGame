#ifndef SYS_SYS_SESSION_H_ 
#define SYS_SYS_SESSION_H_

#include <memory>

/*
================================================
idSession
================================================
*/
class idSession {
public:

	enum class sessionState_t {
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

	idSession() {}

	virtual ~idSession();

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;

	virtual sessionState_t	GetState() const = 0;
};

extern idSession* session;

#endif