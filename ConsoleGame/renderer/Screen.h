#ifndef SCREEN_H
#define SCREEN_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class Screen {
public:
	using pos_type = int;

	/*enum class ConsoleColor {
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
	*/
	struct Pixel {
		Pixel() = default;

		Pixel(char val, int col) : value(val), color(col) {}
		Pixel(const Pixel &p, int col) : value(p.value), color(col) {}
		
		char value;
		int color;
	};

	Screen() = default;
	Screen(pos_type ht, pos_type wd, Pixel back);
	~Screen() = default;
	Screen(const Screen&) = default;
	Screen& operator=(const Screen&) = default;
	Screen(Screen&&) = default;
	Screen& operator=(Screen&&) = default;

	void init();

	inline Screen::Pixel get(pos_type r, pos_type c) const; // explicitly inline

	Screen &set(pos_type row, pos_type col, const Screen::Pixel& ch);
	Screen &set(const Vector2& pos, const Screen::Pixel& ch);
	
	pos_type getHeight() const { return height; }
	pos_type getWidth() const { return width; }

	const Pixel &getBackgroundPixel() const { return backgroundPixel; }

	void clear();
	void clearTextInfo();

	void display();
	void writeInColor(COORD coord, const char* symbol, size_t lenght, int color_text, int color_background = colorNone);
	void writeInColor(const std::string& text, int color_text, int color_background = colorNone);

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
	std::vector<CHAR_INFO> buffer;
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

inline Screen & Screen::set(const Vector2& pos, const Screen::Pixel& ch) {
	return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

#endif
