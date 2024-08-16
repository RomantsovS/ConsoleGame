#ifndef SCREEN_H
#define SCREEN_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class Screen {
public:
    using pos_type = short;
    using color_type = unsigned short;

    struct Pixel {
        Pixel() = default;

        Pixel(char val, color_type col) noexcept : value(val), color(col) {}
        Pixel(const Pixel &p, color_type col) noexcept : value(p.value), color(col) {}

        char value;
        Screen::color_type color;
    };

    Screen() = default;
    Screen(pos_type ht, pos_type wd, Pixel back) noexcept;
    virtual ~Screen() = default;
    Screen(const Screen &) = default;
    Screen &operator=(const Screen &) = default;
    Screen(Screen &&) = default;
    Screen &operator=(Screen &&) = default;

    virtual void init() = 0;

    inline Screen::Pixel get(pos_type r, pos_type c) const noexcept; // explicitly inline

    virtual Screen &set(pos_type row, pos_type col, const Screen::Pixel &ch) = 0;
    Screen &set(const Vector2 &pos, const Screen::Pixel &ch);

    pos_type getWidth() const noexcept { return width; }
    pos_type getHeight() const noexcept { return height; }

    virtual void setBackGroundPixel(const Pixel &pixel) noexcept = 0;
    virtual const Pixel getBackgroundPixel() const noexcept = 0;

    virtual void clear() = 0;
    virtual void clearTextInfo() noexcept = 0;

    virtual void display() noexcept {};
    // void writeInColor(COORD coord, const char* symbol, size_t lenght, Screen::color_type color_text, Screen::color_type color_background = colorNone);
    // void writeInColor(const std::string& text, Screen::color_type color_text, Screen::color_type color_background = colorNone);

    bool readInput(unsigned &key) noexcept;
    std::string waitConsoleInput();

    void writeConsoleOutput(const std::string &text) noexcept;
    void clearConsoleOutut() noexcept;

    virtual void SetConsoleTextTitle(const std::string &str) = 0;

protected:
    pos_type width, height;
};

inline Screen &Screen::set(const Vector2 &pos, const Screen::Pixel &ch) {
    return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

std::unique_ptr<Screen> MakeScreen(Screen::pos_type ht, Screen::pos_type wd, Screen::Pixel back);

#ifdef _WIN32

#include "../sys/win32/win_console_screen.h"

#else

#include "../sys/linux/linux_console_screen.h"

#endif

#endif
