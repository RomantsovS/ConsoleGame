#include <ctime>

/*
================
Sys_Milliseconds
================
*/
clock_t Sys_Milliseconds() {
	static auto sys_timeBase = clock();
	return clock() - sys_timeBase;
}