#include "idlib/precompiled.h"

idCVar window_font_width("window_font_width", "8", CVAR_SYSTEM | CVAR_INIT, "");
idCVar window_font_height("window_font_height", "8", CVAR_SYSTEM | CVAR_INIT,
                          "");
idCVar text_info_max_height("text_info_max_height", "10",
                            CVAR_SYSTEM | CVAR_INIT, "");

WinConsoleScreen::WinConsoleScreen(pos_type wd, pos_type ht,
                                   Pixel back) noexcept
    : Screen(wd, ht) {
  setBackGroundPixel(back);
  h_console_std_out = GetStdHandle(STD_OUTPUT_HANDLE);
  h_console_std_in = GetStdHandle(STD_INPUT_HANDLE);

  if (h_console_std_out == INVALID_HANDLE_VALUE)
    common->FatalError("Bad h_console_std_out");
}

void WinConsoleScreen::init() {
  // Change console visual size to a minimum so ScreenBuffer can shrink
  // below the actual visual size
  // Step 1: shrink window first (required by WinAPI)
  SMALL_RECT window_rect = {0, 0, 0, 0};
  if (!SetConsoleWindowInfo(h_console_std_out, TRUE, &window_rect)) {
    common->FatalError("SetConsoleWindowInfo failed - (%s)\n",
                       getLastErrorMsg());
  }

  // Set the size of the Screen buffer
  COORD coord = {
      static_cast<short>(width),
      static_cast<short>(height + text_info_max_height.GetInteger())};
  if (!SetConsoleScreenBufferSize(h_console_std_out, coord)) {
    common->FatalError("SetConsoleScreenBufferSize failed - (%s)\n",
                       getLastErrorMsg());
  }

  if (!SetConsoleActiveScreenBuffer(h_console_std_out))
    common->FatalError("SetConsoleActiveScreenBuffer  failed - (%s)\n",
                       getLastErrorMsg());

  CONSOLE_CURSOR_INFO cursorInfo;
  GetConsoleCursorInfo(h_console_std_out, &cursorInfo);
  cursorInfo.bVisible = false;  // set the cursor visibility
  SetConsoleCursorInfo(h_console_std_out, &cursorInfo);

  // Get Screen buffer info and check the maximum allowed window size. Return
  // error if exceeded, so user knows their dimensions/fontsize are too large
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(h_console_std_out, &csbi))
    common->FatalError("GetConsoleScreenBufferInfo");
  if (height > csbi.dwMaximumWindowSize.Y)
    common->FatalError("Screen Height / Font Height Too Big. Max %d",
                       csbi.dwMaximumWindowSize.Y);
  if (width > csbi.dwMaximumWindowSize.X)
    common->FatalError("Screen Width / Font Width Too Big");

  // Set Physical Console Window Size
  window_rect = {
      0, 0, static_cast<short>(width - 1),
      static_cast<short>(height + text_info_max_height.GetInteger() - 1)};
  if (!SetConsoleWindowInfo(h_console_std_out, TRUE, &window_rect))
    common->FatalError("SetConsoleWindowInfo wrong width or height");

  // Set flags to allow mouse input
  if (!SetConsoleMode(h_console_std_in, ENABLE_EXTENDED_FLAGS |
                                            ENABLE_WINDOW_INPUT |
                                            ENABLE_MOUSE_INPUT))
    common->FatalError("SetConsoleMode std in");

  // Allocate memory for Screen buffer
  buffer.resize(width * height);
}

WinConsoleScreen& WinConsoleScreen::set(pos_type col, pos_type row,
                                        const Screen::Pixel& ch) {
  if (row >= height || row < 0) {
    return *this;
    common->Error("Screen height: %d out of range: %d", row, height);
  }

  if (col >= width || col < 0) {
    return *this;
    common->Error("Screen width: %d out of range: %d", col, width);
  }

  auto& char_info = buffer[row * width + col];
  char_info.Char.AsciiChar = ch.value;
  char_info.Attributes = ch.color;

  return *this;  // return this object as an lvalue
}

void WinConsoleScreen::clear() {
  std::fill(buffer.begin(), buffer.end(), backgroundPixel);
  recreateBuffer();
}

class idRenderSystem;
extern idRenderSystem* renderSystem;

void WinConsoleScreen::recreateBuffer() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(h_console_std_out, &csbi)) return;

  int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  int h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

  if (w == width && h == height) return;

  width = w;
  height = h;

  buffer.resize(width * height);

  renderSystem->SetWidth(width);
  renderSystem->SetHeight(height);
}

void WinConsoleScreen::display() noexcept {
  SMALL_RECT writeRegion;
  writeRegion.Left = 0;
  writeRegion.Top = 0;
  writeRegion.Right = width - 1;
  writeRegion.Bottom = height - 1;

  COORD bufSize{(SHORT)width, (SHORT)height};
  COORD bufPos{0, 0};

  WriteConsoleOutput(h_console_std_out, buffer.data(), bufSize, bufPos,
                     &writeRegion);
}

void WinConsoleScreen::SetConsoleTextTitle(const std::string& str) {
  SetConsoleTitle(str.c_str());
}
