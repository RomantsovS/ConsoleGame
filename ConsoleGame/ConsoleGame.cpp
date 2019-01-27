#include <iostream>
#include <conio.h>

#include "Screen.h"
#include "Game.h"
#include "Snake.h"

int main()
{
	Screen screen(std::cout, 20, 50, Screen::Pixel(' ', Screen::Black));

	Game Game(screen.getHeight(), screen.getWidth(), 1, 1, Screen::Pixel('#', Screen::White), 200);

	Game.init();

	Game.MainLoop(screen);

	_getch();

	return 0;
}