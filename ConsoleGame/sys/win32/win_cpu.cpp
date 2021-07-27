#pragma hdrstop
#include <precompiled.h>

#include "win_local.h"

/*
================
Sys_GetClockTicks
================
*/
double Sys_GetClockTicks() noexcept {
//#if 0

	LARGE_INTEGER li;

	QueryPerformanceCounter(&li);
	return (double)li.LowPart + (double)0xFFFFFFFF * li.HighPart;
/*
#else

	unsigned long lo, hi;

	__asm {
		push ebx
		xor eax, eax
		cpuid
		rdtsc
		mov lo, eax
		mov hi, edx
		pop ebx
	}
	return (double)lo + (double)0xFFFFFFFF * hi;

#endif*/
}

/*
================
Sys_ClockTicksPerSecond
================
*/
long long Sys_ClockTicksPerSecond() noexcept {
	static long long ticks = 0;

	if (!ticks) {
		LARGE_INTEGER li;
		QueryPerformanceFrequency(&li);
		ticks = li.QuadPart;
	}
	return ticks;
}
