#include <iostream>
#include <conio.h>

#include "Game.h"
//#include "Snake.h"

int main()
{
	Game game(20, 50, 1, 1);

	game.init();

	while (game.isGameRunning())
	{
		game.frame();
	}

	_getch();

	return 0;
}