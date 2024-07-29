#include "idlib/precompiled.h"

#ifdef _WIN32

#include "../sys/win32/win_console_screen.h"

std::unique_ptr<Screen> MakeScreen() {
    return std::make_unique<WinConsoleScreen>();
}

#else

#include "../sys/linux/linux_console_screen.h"
std::unique_ptr<Screen> MakeScreen() {
    return std::make_unique<LinuxConsoleScreen>();
}

#endif
