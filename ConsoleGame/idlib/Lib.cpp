#include "precompiled.h"
#pragma hdrstop

idCommon* idLib::common = nullptr;

/*
===============
idLib::Printf
===============
*/
void idLib::Printf(const char* fmt, ...) {
	va_list		argptr;
	va_start(argptr, fmt);
	if (common && isCommonExists) {
		common->VPrintf(fmt, argptr);
	}
	va_end(argptr);
}

/*
===============
idLib::Warning
===============
*/
void idLib::Warning(const char* fmt, ...) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Warning("%s", text);
}