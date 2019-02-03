#include <iostream>
#include <conio.h>

#include "Game.h"

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