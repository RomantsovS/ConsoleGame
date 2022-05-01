#ifndef IDLIB_SYS_SYS_ASSERT_H_
#define IDLIB_SYS_SYS_ASSERT_H_

/*
================================================================================================

Getting assert() to work as we want on all platforms and code analysis tools can be tricky.

================================================================================================
*/

bool AssertFailed(const std::string& file, int line, const std::string& expression);

// idassert is useful for cases where some external library (think MFC, etc.)
// decides it's a good idea to redefine assert on us
#define idassert( x )	(void)( ( !!( x ) ) || ( AssertFailed( __FILE__, __LINE__, #x ) ) )

// We have the code analysis tools on the 360 compiler,
// so let it know what our asserts are.
// The VS ultimate editions also get it on win32, but not x86
#define assert( x )		__analysis_assume( x ) ; idassert( x )

#endif // ! SYS_ASSERT_H
