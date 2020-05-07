#include <cstdarg>
#include <cstdio>

#include "Str.h"

int idStr::vsnPrintf(char* dest, int size, const char* fmt, va_list argptr)
{
	int ret = vsnprintf_s(dest, size, size, fmt, argptr);

	dest[size - 1] = '\0';

	if (ret < 0 || ret >= size) {
		return -1;
	}
	return ret;
}