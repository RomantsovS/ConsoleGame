#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <iostream>
#include <ctime>
#include <vector>
#include <Windows.h>

class Screen {
public:
    typedef std::string::size_type pos;

	enum ConsoleColor
	{
		Black = 0,
		Blue = 1,
		Green = 2,
		Cyan = 3,
		Red = 4,
		Magenta = 5,
		Brown = 6,
		LightGray = 7,
		DarkGray = 8,
		LightBlue = 9,
		LightGreen = 10,
		LightCyan = 11,
		LightRed = 12,
		LightMagenta = 13,
		Yellow = 14,
		White = 15
	};

	struct Pixel
	{
		char value;
		ConsoleColor color;

		Pixel() = default;

		Pixel(char val, ConsoleColor col) : value(val), color(col) {}
	};

	Screen() = default;
	Screen(pos ht, pos wd, Pixel back);

	~Screen();

	void init();

    inline Screen::Pixel get(pos ht, pos wd) const; // explicitly inline

    Screen &move(pos r, pos c);      // can be made inline later
    Screen &set(Screen::Pixel);
    Screen &set(pos, pos, Screen::Pixel);
	
	pos getHeight() const { return height; }
	pos getWidth() const { return width; }

	const Pixel &getBackgroundSymbol() const { return backgroundPixel; }

	void clear() { clearContents(); }

	Screen &display();
private:
	pos height, width;
	Pixel backgroundPixel;
	char *buffer;
	size_t FPS;

	Pixel *contents;

	void clearContents();
};

Screen::Pixel Screen::get(pos r, pos c) const // declared as inline in the class
{
    pos row = r * width;      // compute row location
    return contents[row + c]; // return character at the given column
}

inline Screen &Screen::set(pos r, pos col, Screen::Pixel ch)
{
	if (r >= height)
		throw std::out_of_range("height out of range");

	if (col >= width)
		throw std::out_of_range("width out of range");

	contents[r*width + col] = ch;  // set specified location to given value

	return *this;                  // return this object as an lvalue
}

void SetColor(Screen::ConsoleColor text, Screen::ConsoleColor background);

#endif