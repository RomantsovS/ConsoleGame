#include "precompiled.h"


idCommon* idLib::common = nullptr;
int idLib::frameNumber = 0;

/*
===============================================================================

	Colors

===============================================================================
*/

Screen::color_type colorNone = -1;
Screen::color_type colorBlack = 0;
Screen::color_type colorBlue = 1;
Screen::color_type colorGreen = 2;
Screen::color_type colorCyan = 3;
Screen::color_type colorRed = 4;
Screen::color_type colorMagenta = 5;
Screen::color_type colorBrown = 6;
Screen::color_type colorLightGray = 7;
Screen::color_type colorDarkGray = 8;
Screen::color_type colorLightBlue = 9;
Screen::color_type colorLightGreen = 10;
Screen::color_type colorLightCyan = 11;
Screen::color_type colorLightRed = 12;
Screen::color_type colorLightMagenta = 13;
Screen::color_type colorYellow = 14;
Screen::color_type colorWhite = 15;

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
idLib::FatalError
===============
*/
void idLib::FatalError(const char* fmt, ...) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->FatalError("%s", text);
}

/*
===============
idLib::Error
===============
*/
void idLib::Error(const char* fmt, ...) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	idStr::vsnPrintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	common->Error("%s", text);
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