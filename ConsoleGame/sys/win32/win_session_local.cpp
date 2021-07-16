#pragma hdrstop
#include <precompiled.h>
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
	idSessionLocalWin() {}
	virtual ~idSessionLocalWin() {}

	// idSessionLocal interface
	virtual void Initialize() override;
	virtual void Shutdown() override;
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
