#include <cassert>
#include <ctime>

#include "sys_public.h"

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

int Sys_Time() {
	return clock();
}