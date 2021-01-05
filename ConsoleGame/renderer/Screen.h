#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>

#include "../idlib/math/Vector2.h"
#include "../framework/Common_local.h"

class Screen {
public:
	using pos_type = SHORT;

	enum class ConsoleColor {
		None = -1,
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

	struct Pixel {
		Pixel() = default;

		Pixel(char val, ConsoleColor col) : value(val), color(col) {}
		Pixel(const Pixel &p, ConsoleColor col) : value(p.value), color(col) {}
		
		char value;
		ConsoleColor color;
	};

	Screen() = default;
	Screen(pos_type ht, pos_type wd, Pixel back);

	~Screen();

	void init();

	inline Screen::Pixel get(pos_type r, pos_type c) const; // explicitly inline

	Screen &set(pos_type r, pos_type col, Screen::Pixel ch);
	Screen &set(Vector2 pos, Screen::Pixel ch);
	
	pos_type getHeight() const { return height; }
	pos_type getWidth() const { return width; }

	const Pixel &getBackgroundPixel() const { return backgroundPixel; }

	void clear();
	void clearTextInfo();

	void display();
	void writeInColor(COORD coord, const char* symbol, size_t lenght, ConsoleColor color_text, ConsoleColor color_background = ConsoleColor::None);
	void writeInColor(const std::string& text, ConsoleColor color_text, ConsoleColor color_background = ConsoleColor::None);

	bool readInput(unsigned& key);
	std::string waitConsoleInput();

	void writeConsoleOutput(const std::string& text);
	void clearConsoleOutut();

	void setDrawOutputBuffer();
	void setStdOutputBuffer();
private:
	pos_type height, width;
	Pixel backgroundPixel;
	//std::vector<char> buffer;
	CHAR_INFO* buffer;
	HANDLE h_console_std_in;
	HANDLE h_console_std_out;
	SMALL_RECT window_rect;

	std::vector<Pixel> contents;
	COORD cur_write_coord;

	void clearContents();
};

Screen::Pixel Screen::get(pos_type r, pos_type c) const // declared as inline in the class
{
	pos_type row = r * width;      // compute row location
	return contents[row + c]; // return character at the given column
}

inline Screen &Screen::set(pos_type r, pos_type col, Screen::Pixel ch) {
	if (r >= height || r < 0)
		common->Error("Screen height: %d out of range: %d", r, height);

	if (col >= width || col < 0)
		common->Error("Screen width: %d out of range: %d", col, width);

	contents[r*width + col] = ch;  // set specified location to given value

	return *this;                  // return this object as an lvalue
}

inline Screen & Screen::set(Vector2 pos, Screen::Pixel ch) {
	return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

#endif
