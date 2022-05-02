message(STATUS "include WindowsPC-GCC")

set(BUILD_CPU_ARCHITECTURE x64)
set(BUILD_PLATFORM Win64)
set(WINDOWS TRUE)

set(MSVC_COMMON_FLAGS 
	#-Wall
	#-Wextra
	#-pedantic
	#-Werror
)
string(REPLACE ";" " " MSVC_COMMON_FLAGS "${MSVC_COMMON_FLAGS}")

set(CMAKE_RC_FLAGS /nologo)

# Override cxx flags
set(CMAKE_CXX_FLAGS "${MSVC_COMMON_FLAGS}" CACHE STRING "C++ Common Flags" FORCE)
#set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "C++ Flags" FORCE)