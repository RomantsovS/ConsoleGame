#ifndef IDLIB_LIB_H_
#define IDLIB_LIB_H_

class idLib {
private:
public:
	static class idCommon* common;
	static int frameNumber;

	// wrapper to idCommon functions 
	static void Printf(const char* fmt, ...);
	static void Error(const char* fmt, ...);
	static void FatalError(const char* fmt, ...);
	static void Warning(const char* fmt, ...);
};

#define MAX_STRING_CHARS		1024		// max length of a string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

// basic colors
extern unsigned short colorNone;
extern unsigned short colorBlack;
extern unsigned short colorWhite;
extern unsigned short colorBlack;
extern unsigned short colorBlue;
extern unsigned short colorGreen;
extern unsigned short colorCyan;
extern unsigned short colorRed;
extern unsigned short colorMagenta;
extern unsigned short colorBrown;
extern unsigned short colorLightGray;
extern unsigned short colorDarkGray;
extern unsigned short colorLightBlue;
extern unsigned short colorLightGreen;
extern unsigned short colorLightCyan;
extern unsigned short colorLightRed;
extern unsigned short colorLightMagenta;
extern unsigned short colorYellow;
extern unsigned short colorWhite;

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
#include "BitMsg.h"
#include "MapFile.h"
#include "Thread.h"

#endif // !IDLIB_LIB_H_
