#ifndef LINUX_CONSOLE_SCREEN_H
#define LINUX_CONSOLE_SCREEN_H

#ifndef _WIN32

extern idCVar window_font_width;
extern idCVar window_font_height;

class LinuxConsoleScreen : public Screen {
public:
	void init();

	inline Screen::Pixel get(pos_type r, pos_type c) const noexcept; // explicitly inline

	Screen &set(pos_type row, pos_type col, const Screen::Pixel& ch);
	Screen &set(const Vector2& pos, const Screen::Pixel& ch);

	void setBackGroundPixel(const Pixel& pixel) noexcept {
	}
	const Pixel getBackgroundPixel() const noexcept { return {}; }

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

	void SetConsoleTextTitle(const std::string& str);
};

#endif

#endif
