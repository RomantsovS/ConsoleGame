#ifndef SYS_WIN32_WIN_LOCAL_H_
#define SYS_WIN32_WIN_LOCAL_H_

#include "../../idlib/precompiled.h"

#include <windows.h>
#include <strsafe.h>

void Sys_QueEvent(sysEventType_t type, int value, int value2, int ptrLength,
                  void* ptr, int inputDeviceNum);

struct Win32Vars_t {
  HANDLE h_console_draw;
  HANDLE h_console_std_out;
  HANDLE h_console_std_in;
};

extern Win32Vars_t win32;

#endif