#include <cassert>
#include <ctime>
#include <strsafe.h>
#include <Windows.h>

#include "../sys_public.h"

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds() {
	static auto sys_timeBase = clock();
	return clock() - sys_timeBase;
}

/*
========================
Sys_Microseconds
========================
*/
long long Sys_Microseconds() {
	static long long ticksPerMicrosecondTimes1024 = 0;

	if (ticksPerMicrosecondTimes1024 == 0) {
		ticksPerMicrosecondTimes1024 = ((long long)Sys_ClockTicksPerSecond() << 10) / 1000000;
		assert(ticksPerMicrosecondTimes1024 > 0);
	}

	return ((long long)((long long)Sys_GetClockTicks() << 10)) / ticksPerMicrosecondTimes1024;
}

long Sys_Time() {
	return clock();
}

char* getLastErrorMsg()
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"), "",
		dw, lpMsgBuf);

	return static_cast<char*>(lpDisplayBuf);
}