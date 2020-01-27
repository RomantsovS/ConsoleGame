#ifndef __SYS_PUBLIC__
#define __SYS_PUBLIC__

// Sys_Milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps
clock_t Sys_Milliseconds();

#endif