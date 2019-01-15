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

int main()
{
	Screen screen(cout, 20, 50, Screen::Pixel(' ', Screen::Black));

	Scene scene(screen.getHeight(), screen.getWidth(), 1, 1, Screen::Pixel('#', Screen::White), 200);

	scene.init();

	scene.MainLoop(screen);

	_getch();

	return 0;
}