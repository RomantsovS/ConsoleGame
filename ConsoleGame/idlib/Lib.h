#ifndef IDLIB_LIB_H_
#define IDLIB_LIB_H_

class idLib {
private:
public:
	static class idCommon* common;

	// wrapper to idCommon functions 
	static void Printf(const char* fmt, ...);
	static void Warning(const char* fmt, ...);
};

#define MAX_STRING_CHARS		1024		// max length of a string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

// basic colors
extern int colorNone;
extern int colorBlack;
extern int colorWhite;
extern int colorBlack;
extern int colorBlue;
extern int colorGreen;
extern int colorCyan;
extern int colorRed;
extern int colorMagenta;
extern int colorBrown;
extern int colorLightGray;
extern int colorDarkGray;
extern int colorLightBlue;
extern int colorLightGreen;
extern int colorLightCyan;
extern int colorLightRed;
extern int colorLightMagenta;
extern int colorYellow;
extern int colorWhite;

/*
===============================================================================

	idLib headers.

===============================================================================
*/

// System
#include "sys/sys_assert.h"

// memory management and arrays
#include "Heap.h"

// math
#include "math/Math.h"
#include "math/Ode.h"
#include "math/Vector2.h"

// bounding volumes
#include "bv/Bounds.h"

// geometry
#include "geometry/TraceModel.h"

// text manipulation
#include <cwchar>
#include "Str.h"
#include "CmdArgs.h"
#include "Token.h"
#include "Lexer.h"

// containers
#include "containers/LinkList.h"
#include "containers/Hierarchy.h"

// misc
#include "Dict.h"
#include "MapFile.h"

#endif // !IDLIB_LIB_H_
