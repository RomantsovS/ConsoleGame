#ifndef WinConsoleScreen_H
#define WinConsoleScreen_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class WinConsoleScreen : public Screen {
public:
    WinConsoleScreen() = default;
    WinConsoleScreen(pos_type ht, pos_type wd, Pixel back) noexcept;
    ~WinConsoleScreen() = default;
    WinConsoleScreen(const WinConsoleScreen &) = default;
    WinConsoleScreen &operator=(const WinConsoleScreen &) = default;
    WinConsoleScreen(WinConsoleScreen &&) = default;
    WinConsoleScreen &operator=(WinConsoleScreen &&) = default;

    void init();

    inline WinConsoleScreen::Pixel get(pos_type r, pos_type c) const noexcept; // explicitly inline

    WinConsoleScreen &set(pos_type row, pos_type col, const WinConsoleScreen::Pixel &ch);
    WinConsoleScreen &set(const Vector2 &pos, const WinConsoleScreen::Pixel &ch);

    pos_type getWidth() const noexcept { return width; }
    pos_type getHeight() const noexcept { return height; }

    void setBackGroundPixel(const Pixel &pixel) noexcept {
        backgroundPixel.Char.AsciiChar = pixel.value;
        backgroundPixel.Attributes = pixel.color;
    }
    const Pixel getBackgroundPixel() const noexcept { return {backgroundPixel.Char.AsciiChar, backgroundPixel.Attributes}; }

    void clear();
    void clearTextInfo() noexcept;

    void display() noexcept;
    // void writeInColor(COORD coord, const char* symbol, size_t lenght, WinConsoleScreen::color_type color_text, WinConsoleScreen::color_type color_background = colorNone);
    // void writeInColor(const std::string& text, WinConsoleScreen::color_type color_text, WinConsoleScreen::color_type color_background = colorNone);

    bool readInput(unsigned &key) noexcept;
    std::string waitConsoleInput();

    void writeConsoleOutput(const std::string &text) noexcept;
    void clearConsoleOutut() noexcept;

    void SetConsoleTextTitle(const std::string &str);

private:
    CHAR_INFO backgroundPixel{};

    // std::vector<char> buffer;
    std::vector<CHAR_INFO> buffer;
    HANDLE h_console_std_in;
    HANDLE h_console_std_out;
    SMALL_RECT window_rect;

    COORD cur_write_coord;
};

inline WinConsoleScreen &WinConsoleScreen::set(const Vector2 &pos, const WinConsoleScreen::Pixel &ch) {
    return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

#endif
