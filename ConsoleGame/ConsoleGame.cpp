#include <iostream>
#include <list>
#include <string>
#include <forward_list>
#include <algorithm>
#include <functional>
#include <iterator>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <regex>
#include <random>
#include <conio.h>

#include "Screen.h"
#include "Scene.h"
#include "Snake.h"

using std::cout;
using std::cin;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::list;
using std::forward_list;
using std::map;
using std::set;

void MainLoop(Screen &screen, Scene &scene);

int main()
{
	Screen screen(cout, 20, 50, ' ');

	Scene scene(screen.getHeight(), screen.getWidth(), 1, 1, '#');

	scene.init();

	MainLoop(screen, scene);

	system("pause");

	return 0;
}

void breakTime(int seconds)
{
	clock_t temp;
	temp = clock() + seconds * CLOCKS_PER_SEC;
	while (clock() < temp)
	{
	}
}

void MainLoop(Screen &screen, Scene &scene)
{
	char c;

	bool gameRunning = true;

	while(gameRunning)
	{
		system("cls");

		screen.clear();
		scene.fillBorder(screen);

		try
		{
			scene.drawToScreen(screen);
		}
		catch (std::exception &err)
		{
			cout << err.what() << endl
				<< "press ane key to continue...\n";
			_getch();
		}

		screen.display();

		c = _getch();

		switch (c)
		{
		case 27:
			gameRunning = false;

			system("cls");

			cout << "enter Q to quit or any key to continue: ";

			cin >> c;

			if (c == 'Q' || c == 'q')
				break;

			gameRunning = true;
		default:
			scene.onKeyPressed(c);
			;
		}

		scene.onEvent();
	}
}