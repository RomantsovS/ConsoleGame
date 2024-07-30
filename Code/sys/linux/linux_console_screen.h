#ifndef LINUX_CONSOLE_SCREEN_H
#define LINUX_CONSOLE_SCREEN_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class LinuxConsoleScreen : public Screen {
public:
	LinuxConsoleScreen(pos_type ht, pos_type wd, Pixel back) noexcept;
	~LinuxConsoleScreen() = default;

	void init() override;

	inline Screen::Pixel get(pos_type r, pos_type c) const noexcept; // explicitly inline

	Screen &set(pos_type row, pos_type col, const Screen::Pixel& ch) override;

	void setBackGroundPixel(const Pixel& pixel) noexcept override {
	}
	const Pixel getBackgroundPixel() const noexcept override { return {}; }

	void clear() override;
	void clearTextInfo() noexcept override;

	void display() noexcept override;

	void SetConsoleTextTitle(const std::string& str) override;
private:
	std::vector<char> buffer;
};

#endif
