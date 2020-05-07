#ifndef IDLIB_TEXT_STR_H_
#define IDLIB_TEXT_STR_H_

class idStr {

public:
	static int vsnPrintf(char* dest, int size, const char* fmt, va_list argptr);
};

#endif // !IDLIB_TEXT_STR_H_
