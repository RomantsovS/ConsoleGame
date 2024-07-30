#include "idlib/precompiled.h"
#include "linux_console_screen.h"

idCVar window_font_width("window_font_width", "8", CVAR_SYSTEM | CVAR_INIT, "");
idCVar window_font_height("window_font_height", "8", CVAR_SYSTEM | CVAR_INIT,
	"");
idCVar text_info_max_height("text_info_max_height", "10",
	CVAR_SYSTEM | CVAR_INIT, "");

LinuxConsoleScreen::LinuxConsoleScreen(pos_type wd, pos_type ht, Pixel back) noexcept
	: Screen(wd, ht, back) {
		setBackGroundPixel(back);
}

void LinuxConsoleScreen::init() {
	buffer.resize(width * height);
}

Screen &LinuxConsoleScreen::set(pos_type row, pos_type col, const Screen::Pixel &ch) {
    if (row >= height || row < 0) {
		return *this;
		common->Error("Screen height: %d out of range: %d", row, height);
	}

	if (col >= width || col < 0) {
		return *this;
		common->Error("Screen width: %d out of range: %d", col, width);
	}

	buffer[row * width + col] = ch.value;

	return *this;  // return this object as an lvalue
}

void LinuxConsoleScreen::clear() {
}

void LinuxConsoleScreen::clearTextInfo() noexcept {
}

void LinuxConsoleScreen::display() noexcept {
}

void LinuxConsoleScreen::SetConsoleTextTitle(const std::string &str) {
}
