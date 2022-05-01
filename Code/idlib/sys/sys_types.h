#ifndef IDLIB_SYS_SYS_TYPES_H_
#define IDLIB_SYS_SYS_TYPES_H_

typedef long long			int64;

#ifndef BIT
#define BIT( num )				( 1ULL << ( num ) )
#endif

#define	max_string_chars		1024		// max length of a static string
#define MAX_PRINT_MSG			16384		// buffer size for our various printf routines

#endif // ! SYS_TYPES_H
