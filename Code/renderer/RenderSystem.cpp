#include "idlib/precompiled.h"

#include "tr_local.h"

idRenderSystemLocal tr;
idRenderSystem* renderSystem = &tr;

const int BIGCHAR_WIDTH = 8;
const int BIGCHAR_HEIGHT = 8;

/*
====================
RenderCommandBuffers
====================
*/
void idRenderSystemLocal::RenderCommandBuffers() {
  // r_skipBackEnd allows the entire time of the back end
  // to be removed from performance measurements, although
  // nothing will be drawn to the screen->  If the prints
  // are going to a file, or r_skipBackEnd is later disabled,
  // usefull data can be received.

  if (!tr.update_frame) return;

  // r_skipRender is usually more usefull, because it will still
  // draw 2D graphics
  if (!r_skipBackEnd.GetBool()) {
    RB_ExecuteBackEndCommands();
  }
}

idRenderSystem::~idRenderSystem() {}

idRenderSystemLocal::idRenderSystemLocal() { Clear(); }

idRenderSystemLocal::~idRenderSystemLocal() {}

/*
=============
idRenderSystemLocal::SetColor
=============
*/
void idRenderSystemLocal::SetColor(const Screen::color_type rgba) noexcept {
  currentColorNativeBytesOrder = rgba;
}

/*
=============
idRenderSystemLocal::DrawStretchPic
=============
*/
void idRenderSystemLocal::DrawStretchPic(int x, int y, int w, int h, int s1,
                                         int t1, const idMaterial* material) {
  if (!R_IsInitialized()) {
    return;
  }
  if (material == nullptr) {
    return;
  }

  auto image = material->GetStage()->image;
  if (!image->IsLoaded()) {
    common->Error("Image %s is not loaded", image->GetName().c_str());
  }
  const auto& imagePixels = image->GetPixels();

  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      int pixelIndex = t1 * BIGCHAR_WIDTH * BIGCHAR_WIDTH * 16 +
                       i * BIGCHAR_WIDTH * 16 + s1 * BIGCHAR_WIDTH + j;
      try {
        auto pixel = imagePixels.at(pixelIndex).screenPixel;

        if (pixel.color != colorBlack)
          pixel.color = currentColorNativeBytesOrder;

        tr.screen->set(x + j, y + i, pixel);
      } catch (const std::exception& err) {
        common->Error(err.what());
      }
    }
  }
}

/*
=====================
idRenderSystemLocal::DrawBigChar
=====================
*/
void idRenderSystemLocal::DrawBigChar(int x, int y, int ch) {
  int row{}, col{};

  ch &= 255;

  if (ch == ' ') {
    return;
  }

  if (y < -BIGCHAR_HEIGHT) {
    return;
  }

  col = ch % 16;
  row = (ch - ' ') / 16;

  DrawStretchPic(x, y, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, col, row,
                 charSetMaterial.get());
}

/*
==================
idRenderSystemLocal::DrawBigStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void idRenderSystemLocal::DrawBigStringExt(int x, int y,
                                           std::string_view string,
                                           const Screen::color_type setColor,
                                           bool forceColor) {
  // idVec4		color;
  int xx;

  // draw the colored text
  xx = x;
  SetColor(setColor);
  for (auto s : string) {
    if (s == '\0') break;
    DrawBigChar(xx, y, s);
    xx += BIGCHAR_WIDTH;
  }
  SetColor(colorWhite);
}

void idRenderSystemLocal::DrawString(Vector2 pos,
                                               const std::string& str,
                                               Screen::color_type color) {
  for (const auto& ch : str) {
    screen->set(pos, Screen::Pixel(ch, color));
    ++pos.x;
  }
}

std::shared_ptr<idRenderWorld> idRenderSystemLocal::AllocRenderWorld() {
  auto rw = std::make_shared<idRenderWorldLocal>();

  worlds.push_back(rw);
  return std::dynamic_pointer_cast<idRenderWorld>(rw);
}

void idRenderSystemLocal::FreeRenderWorld(std::shared_ptr<idRenderWorld> rw) {
  auto iter = std::find(worlds.begin(), worlds.end(), rw);

  if (iter != worlds.end()) worlds.erase(iter);
}

void idRenderSystemLocal::Display() noexcept { screen->display(); }

void idRenderSystemLocal::ClearScreen() { screen->clear(); }

void idRenderSystemLocal::UpdateTimers() {
  static auto prev_frame_update_time = Sys_Milliseconds();

  auto t = Sys_Milliseconds();

  if (t - prev_frame_update_time > update_frame_time) {
    tr.update_frame = true;
    prev_frame_update_time = t;
  }

  if (!tr.update_frame) return;

  tr.ClearScreen();
}

void idRenderSystemLocal::SetConsoleTextTitle(const std::string& str) {
  screen->SetConsoleTextTitle(str);
}
