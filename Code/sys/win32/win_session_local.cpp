#include "idlib/precompiled.h"
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
	void Initialize() noexcept override;
	void Shutdown() noexcept override;
};

idSessionLocalWin sessionLocalWin;
idSession* session = &sessionLocalWin;

/*
========================
idSessionLocalWin::Initialize
========================
*/
void idSessionLocalWin::Initialize() noexcept {
	idSessionLocal::Initialize();
}

/*
========================
idSessionLocalWin::Shutdown
========================
*/
void idSessionLocalWin::Shutdown() noexcept {
	idSessionLocal::Shutdown();
}
