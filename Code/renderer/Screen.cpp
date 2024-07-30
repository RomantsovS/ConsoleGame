#include "idlib/precompiled.h"

Screen::Screen(pos_type wd, pos_type ht, Pixel back) noexcept
	: width(wd),
	height(ht) {
}

#ifdef _WIN32

#include "../sys/win32/win_console_screen.h"

std::unique_ptr<Screen> Screen::MakeScreen(pos_type ht, pos_type wd, Pixel back) {
    return std::make_unique<WinConsoleScreen>(ht, wd, back);
}

#else

#include "../sys/linux/linux_console_screen.h"
std::unique_ptr<Screen> Screen::MakeScreen(pos_type ht, pos_type wd, Pixel back) {
    return std::make_unique<LinuxConsoleScreen>(ht, wd, back);
}

#endif
