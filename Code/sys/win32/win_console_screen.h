#ifndef WinConsoleScreen_H
#define WinConsoleScreen_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class WinConsoleScreen : public Screen {
 public:
  WinConsoleScreen() = default;
  WinConsoleScreen(pos_type ht, pos_type wd, Pixel back) noexcept;
  WinConsoleScreen(const WinConsoleScreen&) = default;
  WinConsoleScreen(WinConsoleScreen&&) = default;
  ~WinConsoleScreen() = default;
  WinConsoleScreen& operator=(const WinConsoleScreen&) = default;
  WinConsoleScreen& operator=(WinConsoleScreen&&) = default;

  void init();

  WinConsoleScreen& set(pos_type row, pos_type col,
                        const WinConsoleScreen::Pixel& ch);
  WinConsoleScreen& set(const Vector2& pos, const Screen::Pixel& ch);

  pos_type getWidth() const noexcept { return width; }
  pos_type getHeight() const noexcept { return height; }

  void setBackGroundPixel(const Pixel& pixel) noexcept {
    backgroundPixel.Char.UnicodeChar = pixel.value;
    backgroundPixel.Attributes = pixel.color;
  }
  const Pixel getBackgroundPixel() const noexcept {
    return {backgroundPixel.Char.UnicodeChar, backgroundPixel.Attributes};
  }

  void clear();

  void display() noexcept;

  void SetConsoleTextTitle(const std::string& str);

 private:
  CHAR_INFO backgroundPixel{};

  // std::vector<char> buffer;
  std::vector<CHAR_INFO> buffer;
  HANDLE h_console_std_in;
  HANDLE h_console_std_out;

  void recreateBuffer();
};

inline WinConsoleScreen& WinConsoleScreen::set(const Vector2& pos,
                                               const Screen::Pixel& ch) {
  return set(static_cast<pos_type>(pos.x), static_cast<pos_type>(pos.y), ch);
}

class WinConsoleScreenFactory : ConsoleScreenFactory {
 public:
  std::unique_ptr<Screen> MakeScreen(Screen::pos_type ht, Screen::pos_type wd,
                                     Screen::Pixel back) const override {
    return std::make_unique<WinConsoleScreen>(ht, wd, back);
  }
};


#endif
