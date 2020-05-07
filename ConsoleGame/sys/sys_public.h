#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

#include <string>

void Sys_Error(const std::string& error, ...);
void Sys_Quit();

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
int Sys_Milliseconds();
long long Sys_Microseconds();

int Sys_Time();

// for accurate performance testing
double Sys_GetClockTicks();
long long Sys_ClockTicksPerSecond();

#endif