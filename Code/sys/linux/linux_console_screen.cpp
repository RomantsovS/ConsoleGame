#include "idlib/precompiled.h"

idCVar window_font_width("window_font_width", "8", CVAR_SYSTEM | CVAR_INIT, "");
idCVar window_font_height("window_font_height", "8", CVAR_SYSTEM | CVAR_INIT,
                          "");
idCVar text_info_max_height("text_info_max_height", "10",
                            CVAR_SYSTEM | CVAR_INIT, "");

LinuxConsoleScreen::LinuxConsoleScreen(pos_type wd, pos_type ht, Pixel back) noexcept
    : Screen(wd, ht) {
    setBackGroundPixel(back);
}

void LinuxConsoleScreen::init() {
    buffer.resize(width * height);
}

Screen &LinuxConsoleScreen::set(pos_type col, pos_type row, const Screen::Pixel &ch) {
    if (row >= height || row < 0) {
        common->Warning("Screen height: %d out of range: %d", row, height);
        return *this;
    }

    if (col >= width || col < 0) {
        common->Warning("Screen width: %d out of range: %d", col, width);
        return *this;
    }

    if(ch.color == colorBlack) {
        return *this;
    }

    buffer[row * width + col] = ch.value == -37 ? '#' : ch.value;

    return *this; // return this object as an lvalue
}

void LinuxConsoleScreen::clear() {
    std::system("clear");
    std::fill(buffer.begin(), buffer.end(), backgroundPixel.value);
}

void LinuxConsoleScreen::clearTextInfo() noexcept {
}

void LinuxConsoleScreen::display() noexcept {
    for (pos_type i = 0; i < height; ++i) {
        for (pos_type j = 0; j < width; ++j) {
            std::cout << buffer[i * width + j];
        }
        std::cout << '\n';
    }
}

void LinuxConsoleScreen::SetConsoleTextTitle(const std::string &str) {
}

std::unique_ptr<Screen> MakeScreen(Screen::pos_type ht, Screen::pos_type wd, Screen::Pixel back) {
    return std::make_unique<LinuxConsoleScreen>(ht, wd, back);
}