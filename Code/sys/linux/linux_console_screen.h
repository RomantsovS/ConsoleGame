#ifndef LINUX_CONSOLE_SCREEN_H
#define LINUX_CONSOLE_SCREEN_H

extern idCVar window_font_width;
extern idCVar window_font_height;

class LinuxConsoleScreen : public Screen {
 public:
  LinuxConsoleScreen(pos_type ht, pos_type wd, Pixel back) noexcept;
  ~LinuxConsoleScreen() = default;

  void init() override;

  Screen& set(pos_type col, pos_type row, const Screen::Pixel& ch) override;

  void setBackGroundPixel(const Pixel& pixel) noexcept override {
    backgroundPixel = pixel;
  }
  const Pixel getBackgroundPixel() const noexcept override {
    return backgroundPixel;
  }

  void clear() override;

  void display() noexcept override;

  void SetConsoleTextTitle(const std::string& str) override;

 private:
  std::vector<char> buffer;
  Pixel backgroundPixel;
};

class LinuxConsoleScreenFactory : ConsoleScreenFactory {
 public:
  std::unique_ptr<Screen> MakeScreen(Screen::pos_type ht, Screen::pos_type wd,
                                     Screen::Pixel back) const override {
    return std::make_unique<LinuxConsoleScreen>(ht, wd, back);
  }
};

#endif
