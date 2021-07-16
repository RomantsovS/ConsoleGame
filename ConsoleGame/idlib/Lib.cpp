#include "precompiled.h"
#pragma hdrstop

idCommon* idLib::common = nullptr;

/*
===============================================================================

	Colors

===============================================================================
*/

int colorNone = -1;
int colorBlack = 0;
int colorBlue = 1;
int colorGreen = 2;
int colorCyan = 3;
int colorRed = 4;
int colorMagenta = 5;
int colorBrown = 6;
int colorLightGray = 7;
int colorDarkGray = 8;
int colorLightBlue = 9;
int colorLightGreen = 10;
int colorLightCyan = 11;
int colorLightRed = 12;
int colorLightMagenta = 13;
int colorYellow = 14;
int colorWhite = 15;

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