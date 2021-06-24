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