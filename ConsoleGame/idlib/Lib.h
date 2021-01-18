#ifndef IDLIB_LIB_H_
#define IDLIB_LIB_H_

#define	max_string_chars		1024		// max length of a static string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

/*
===============================================================================

	idLib headers.

===============================================================================
*/

// System
#include "sys/sys_assert.h"

// memory management and arrays

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

// containers
#include "containers/LinkList.h"
#include "containers/Hierarchy.h"

// misc
#include "Dict.h"

#endif // !IDLIB_LIB_H_
