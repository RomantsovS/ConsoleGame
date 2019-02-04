#include <iostream>
#include <conio.h>

#include "Game_local.h"

int main()
{
	gameLocal.Init();

	while (gameLocal.IsGameRunning())
	{
		gameLocal.Frame();
	}

	_getch();

	return 0;
}