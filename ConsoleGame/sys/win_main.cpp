#include <iostream>
#include <conio.h>
#include <Windows.h>

#include "sys_public.h"
#include "../framework/Common_local.h"

/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void Sys_Error(const std::string& error, ...) {

	exit(EXIT_FAILURE);
}

/*
==============
Sys_Quit
==============
*/
void Sys_Quit() {
	//exit(EXIT_SUCCESS);
	ExitProcess(0);
}

int main(int argc, const char * const * argv, const char * cmdline)
{
	// get the initial time base
	Sys_Milliseconds();

	common->Init(argc, argv, cmdline);

	// main game loop
	while (1)
	{
		// run the game
		common->Frame();
	}

	_getch();

	return 0;
}