#include "idlib/precompiled.h"

#include "../../renderer/tr_local.h"

int main(int argc, const char * const * argv) {
	// get the initial time base
	Sys_Milliseconds();

	common->Init(argc, argv, nullptr);

	// main game loop
	while (1) {
		// run the game
		common->Frame();
	}

	// never gets here
	return 0;
}
