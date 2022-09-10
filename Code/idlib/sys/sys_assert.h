#ifndef IDLIB_SYS_SYS_idassert_H_
#define IDLIB_SYS_SYS_idassert_H_

/*
================================================================================================

Getting idassert() to work as we want on all platforms and code analysis tools can be tricky.

================================================================================================
*/

bool idassertFailed(const std::string& file, int line, const std::string& expression);

// ididassert is useful for cases where some external library (think MFC, etc.)
// decides it's a good idea to redefine idassert on us
#define ididassert( x )	(void)( ( !!( x ) ) || ( idassertFailed( __FILE__, __LINE__, #x ) ) )

// We have the code analysis tools on the 360 compiler,
// so let it know what our idasserts are.
// The VS ultimate editions also get it on win32, but not x86
#define idassert( x )		__analysis_assume( x ) ; ididassert( x )

#define verify( x )		( ( x ) ? true : ( idassertFailed( __FILE__, __LINE__, #x ), false ) )

#endif // ! SYS_idassert_H
