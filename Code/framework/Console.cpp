#include "idlib/precompiled.h"

#include "Common_local.h"
#include "../renderer/tr_local.h"

const size_t CON_TEXTSIZE = 1024;

// the console will query the cvar and command systems for
// command completion information

class idConsoleLocal : public idConsole {
 public:
  void Init() noexcept override;
  void Shutdown() noexcept override;
  bool ProcessEvent(const sysEvent_t* event, bool forceAccept) override;
  bool Active() noexcept override;
  // virtual	void		ClearNotifyLines();
  void Close() noexcept override;
  void Print(const char* text) override;
  void Draw(bool forceFullScreen) override;

  // void				Dump(const char* toFile);
  void Clear() noexcept;

 private:
  void KeyDownEvent(int key);

  void Linefeed();

  void Bottom();

  void DrawInput();
  void DrawSolidConsole(float frac);

  int DrawFPS(int y);

  //============================

  int LINE_WIDTH;
  int TOTAL_LINES;

  bool keyCatching;

  std::string text;
  int current;  // line where next message will be printed
  int x;        // offset in current line for next print
  int display;  // bottom of console displays this line

  int vislines;  // in scanlines

  idEditField consoleField;
};

static idConsoleLocal localConsole;
idConsole* console = &localConsole;

constexpr int FPS_FRAMES = 8;

int idConsoleLocal::DrawFPS(int y) {
  static long long previousTimes[FPS_FRAMES];
  static int index;
  static long long previous;

  // don't use serverTime, because that will be drifting to
  // correct for internet lag changes, timescales, timedemos, etc
  auto t = Sys_Microseconds();
  auto frameTime = t - previous;
  previous = t;

  previousTimes[index % FPS_FRAMES] = frameTime;
  index++;

  int fps = 0;

  if (index > FPS_FRAMES) {
    // average multiple frames together to smooth changes out a bit
    long long total = 0;
    for (int i = 0; i < FPS_FRAMES; i++) {
      total += previousTimes[i];
    }
    if (!total) {
      total = 1;
    }
    fps = static_cast<int>(1000000000ll * FPS_FRAMES / total);
    fps = (fps + 500) / 1000;
  }

  if (tr.update_frame) {
    renderSystem->SetConsoleTextTitle(
        string_format("ConsoleGame %6d fps, %10.0f ftime, %6d gtime", fps,
                      MS2SEC(frameTime), game->GetTime()));
  }

  return y;
}

/*
==============
Con_Clear_f
==============
*/
static void Con_Clear_f(const idCmdArgs& args) { localConsole.Clear(); }

/*
==============
idConsoleLocal::Init
==============
*/
void idConsoleLocal::Init() noexcept {
  keyCatching = false;

  LINE_WIDTH = 40;
  TOTAL_LINES = CON_TEXTSIZE / LINE_WIDTH;
  text.resize(CON_TEXTSIZE);

  consoleField.Clear();

  cmdSystem->AddCommand("clear", Con_Clear_f, CMD_FL_SYSTEM,
                        "clears the console");
}

/*
==============
idConsoleLocal::Shutdown
==============
*/
void idConsoleLocal::Shutdown() noexcept {}

/*
================
idConsoleLocal::Active
================
*/
bool idConsoleLocal::Active() noexcept { return keyCatching; }

/*
================
idConsoleLocal::Close
================
*/
void idConsoleLocal::Close() noexcept { keyCatching = false; }

/*
================
idConsoleLocal::Clear
================
*/
void idConsoleLocal::Clear() noexcept {
  std::fill(text.begin(), text.end(), ' ');

  Bottom();  // go to end
}

/*
================
idConsoleLocal::Bottom
================
*/
void idConsoleLocal::Bottom() { display = current; }

/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
KeyDownEvent

Handles history and console scrollback
====================
*/
void idConsoleLocal::KeyDownEvent(int key) {
  // Execute F key bindings
  if (key >= static_cast<int>(keyNum_t::K_F1) &&
      key <= static_cast<int>(keyNum_t::K_F12)) {
    idKeyInput::ExecKeyBinding(key);
    return;
  }

  // ctrl-L clears screen
  if (key == static_cast<int>(keyNum_t::K_L) &&
      (idKeyInput::IsDown(static_cast<int>(keyNum_t::K_LCTRL)) ||
       idKeyInput::IsDown(static_cast<int>(keyNum_t::K_RCTRL)))) {
    Clear();
    return;
  }

  // enter finishes the line
  if (key == static_cast<int>(keyNum_t::K_ENTER) ||
      key == static_cast<int>(keyNum_t::K_KP_ENTER)) {
    common->Printf(">%s\n", consoleField.GetBuffer());

    cmdSystem->BufferCommandText(CMD_EXEC_APPEND,
                                 consoleField.GetBuffer());  // valid command
    cmdSystem->BufferCommandText(CMD_EXEC_APPEND, "\n");

    // copy line to history buffer

    consoleField.Clear();
    return;
  }

  // pass to the normal editline routine
  consoleField.KeyDownEvent(key);
}

/*
==============
ProcessEvent
==============
*/
bool idConsoleLocal::ProcessEvent(const sysEvent_t* proc_event,
                                  bool forceAccept) {
  const bool consoleKey =
      proc_event->evType == SE_CHAR &&
      proc_event->evValue == static_cast<int>(keyNum_t::K_GRAVE) &&
      com_allowConsole.GetBool();

  // we always catch the console key event
  if (!forceAccept && consoleKey) {
    // ignore up events
    if (proc_event->evValue2 == 0) {
      return true;
    }

    // a down event will toggle the destination lines
    if (keyCatching) {
      Close();
    } else {
      consoleField.Clear();
      keyCatching = true;
    }
    return true;
  }

  // if we aren't key catching, dump all the other events
  if (!forceAccept && !keyCatching) {
    return false;
  }

  // handle key and character events
  if (proc_event->evType == SE_CHAR) {
    // never send the console key as a character
    if (proc_event->evValue != '`' && proc_event->evValue != '~' &&
        proc_event->evValue2 == 1) {
      consoleField.CharEvent(proc_event->evValue);
    }
    return true;
  }

  if (proc_event->evType == SE_KEY) {
    // ignore up key events
    if (proc_event->evValue2 == 0) {
      return true;
    }

    KeyDownEvent(proc_event->evValue);
    return true;
  }

  // we don't handle things like mouse, joystick, and network packets
  return false;
}

/*
===============
Linefeed
===============
*/
void idConsoleLocal::Linefeed() {
  int i;

  x = 0;
  if (display == current) {
    display++;
  }
  current++;
  for (i = 0; i < LINE_WIDTH; i++) {
    int offset = ((unsigned int)current % TOTAL_LINES) * LINE_WIDTH + i;
    text[offset] = ' ';
  }
}

/*
================
Print

Handles cursor positioning, line wrapping, etc
================
*/
void idConsoleLocal::Print(const char* txt) {
  int y;
  int c, l;

  if (TOTAL_LINES == 0) {
    // not yet initialized
    return;
  }

  while ((c = *(const unsigned char*)txt) != 0) {
    y = current % TOTAL_LINES;

    // if we are about to print a new word, check to see
    // if we should wrap to the new line
    if (c > ' ' && (x == 0 || text[y * LINE_WIDTH + x - 1] <= ' ')) {
      // count word length
      for (l = 0; l < LINE_WIDTH; l++) {
        if (txt[l] <= ' ') {
          break;
        }
      }

      // word wrap
      if (l != LINE_WIDTH && (x + l >= LINE_WIDTH)) {
        Linefeed();
        ++y;
      }
    }

    txt++;

    switch (c) {
      case '\n':
        Linefeed();
        break;
      case '\t':
        do {
          text[y * LINE_WIDTH + x] = ' ';
          x++;
          if (x >= LINE_WIDTH) {
            Linefeed();
            x = 0;
          }
        } while (x & 3);
        break;
      case '\r':
        x = 0;
        break;
      default:  // display character and advance
        text[y * LINE_WIDTH + x] = c;
        x++;
        if (x >= LINE_WIDTH) {
          Linefeed();
          x = 0;
        }
        break;
    }
  }
}

/*
================
DrawInput

Draw the editline after a ] prompt
================
*/
void idConsoleLocal::DrawInput() {
  std::string console_text = std::string(":").append(consoleField.GetBuffer());
  console_text.append("_");
  renderSystem->DrawString(Vector2(0, vislines), console_text, colorWhite);
}

/*
================
DrawSolidConsole

Draws the console with the solid background
================
*/
void idConsoleLocal::DrawSolidConsole(float frac) {
  int lines = renderSystem->GetHeight() * frac;
  if (lines <= 0) {
    return;
  }

  if (lines > renderSystem->GetHeight()) {
    lines = renderSystem->GetHeight();
  }

  int y = frac * renderSystem->GetHeight() - 2;
  if (y < 0.0f) {
    y = 0.0f;
  }

  vislines = lines;
  int rows = (lines - BIGCHAR_WIDTH) / BIGCHAR_WIDTH;  // rows of text to draw

  y = lines - (BIGCHAR_HEIGHT * 0);

  size_t row = display;

  auto text_sv = std::string_view(text);

  for (int i = 0; i < lines && row >= 0; i++, y -= BIGCHAR_HEIGHT, row--) {
    size_t text_p = (row % TOTAL_LINES) * LINE_WIDTH;

    renderSystem->DrawString(Vector2(x, y), text_sv.substr(text_p, LINE_WIDTH),
                             colorWhite);
  }

  // draw the input prompt, user text, and cursor if desired
  DrawInput();

  if (keyCatching) {
     RB_ShowDebugText();
  }
}

/*
==============
Draw

ForceFullScreen is used by the editor
==============
*/
void idConsoleLocal::Draw(bool forceFullScreen) {
  if (forceFullScreen) {
    // if we are forced full screen because of a disconnect,
    // we want the console closed when we go back to a session state
    Close();
    // we are however catching keyboard input
    keyCatching = true;
  }

  DrawSolidConsole(keyCatching ? 0.8f : 0.0f);

  if (com_showFPS.GetBool()) {
    DrawFPS(0);
  }
}