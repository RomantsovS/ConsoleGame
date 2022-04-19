#ifndef SCREEN_H
#define SCREEN_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class Screen {
public:
	using pos_type = int;
	using color_type = unsigned short;

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

		Pixel(char val, color_type col) noexcept : value(val), color(col) {}
		Pixel(const Pixel &p, color_type col) noexcept : value(p.value), color(col) {}
		
		char value;
		Screen::color_type color;
	};

	Screen() = default;
	Screen(pos_type ht, pos_type wd, Pixel back) noexcept;
	~Screen() = default;
	Screen(const Screen&) = default;
	Screen& operator=(const Screen&) = default;
	Screen(Screen&&) = default;
	Screen& operator=(Screen&&) = default;

	void init();

	inline Screen::Pixel get(pos_type r, pos_type c) const noexcept; // explicitly inline

	Screen &set(pos_type row, pos_type col, const Screen::Pixel& ch);
	Screen &set(const Vector2& pos, const Screen::Pixel& ch);
	
	pos_type getWidth() const noexcept { return width; }
	pos_type getHeight() const noexcept { return height; }

	const Pixel &getBackgroundPixel() const noexcept { return backgroundPixel; }

	void clear();
	void clearTextInfo() noexcept;

	void display() noexcept;
	//void writeInColor(COORD coord, const char* symbol, size_t lenght, Screen::color_type color_text, Screen::color_type color_background = colorNone);
	//void writeInColor(const std::string& text, Screen::color_type color_text, Screen::color_type color_background = colorNone);

	bool readInput(unsigned& key) noexcept;
	std::string waitConsoleInput();

	void writeConsoleOutput(const std::string& text) noexcept;
	void clearConsoleOutut() noexcept;

	void setDrawOutputBuffer();
	void setStdOutputBuffer();
private:
	pos_type width, height;
	Pixel backgroundPixel;
	//std::vector<char> buffer;
	std::vector<CHAR_INFO> buffer;
	HANDLE h_console_std_in;
	HANDLE h_console_std_out;
	SMALL_RECT window_rect;

	COORD cur_write_coord;
};

inline Screen& Screen::set(const Vector2& pos, const Screen::Pixel& ch) {
	return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

#endif
