#include <iostream>
#include <conio.h>

#include "Game.h"

int main()
{
	Game game;

	game.init();

	while (game.isGameRunning())
	{
		game.frame();
	}

	_getch();

	return 0;
}