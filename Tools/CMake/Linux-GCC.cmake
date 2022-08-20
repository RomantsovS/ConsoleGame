message(STATUS "include Linux-GCC")

set(BUILD_CPU_ARCHITECTURE x64)
set(BUILD_PLATFORM Linux)
set(WINDOWS FALSE)

set(GCC_COMMON_FLAGS 
	#-Wall
	#-Wextra
	#-pedantic
	#-Werror
)
string(REPLACE ";" " " GCC_COMMON_FLAGS "${GCC_COMMON_FLAGS}")

set(CMAKE_RC_FLAGS /nologo)

# Override cxx flags
set(CMAKE_CXX_FLAGS "${GCC_COMMON_FLAGS}" CACHE STRING "C++ Common Flags" FORCE)