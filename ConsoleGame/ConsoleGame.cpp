#include <iostream>
#include <conio.h>

#include "Common_local.h"

int main(int argc, const char * const * argv, const char * cmdline)
{
	commonLocal.Init(argc, argv, cmdline);

	while (commonLocal.IsGameRunning())
	{
		commonLocal.Frame();
	}

	commonLocal.Quit();

	_getch();

	return 0;
}