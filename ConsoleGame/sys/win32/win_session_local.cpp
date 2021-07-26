#include <precompiled.h>
#pragma hdrstop

#include "../../framework/Common_local.h"
#include "../sys_session_local.h"
#include "win_local.h"

/*
========================
idSessionLocalWin::idSessionLocalWin
========================
*/
class idSessionLocalWin : public idSessionLocal {
public:
	// idSessionLocal interface
	void Initialize() override;
	void Shutdown() override;
};

idSessionLocalWin sessionLocalWin;
idSession* session = &sessionLocalWin;

/*
========================
idSessionLocalWin::Initialize
========================
*/
void idSessionLocalWin::Initialize() {
	idSessionLocal::Initialize();
}

/*
========================
idSessionLocalWin::Shutdown
========================
*/
void idSessionLocalWin::Shutdown() {
	idSessionLocal::Shutdown();
}
