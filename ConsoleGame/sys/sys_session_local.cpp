#include "sys_session.h"

/*
========================
idSessionLocal::~idSession
========================
*/
idSession::~idSession() {
	signInManager = nullptr;
}