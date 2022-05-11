set(MSVC_COMMON_FLAGS 
	/nologo     # Don't show version info
	/Wall         # Enable warning level
	/fp:fast    # Use fast floating point precision model
	/Zc:wchar_t # Parse wchar_t as internal type
	/GF         # Eliminate Duplicate Strings
	/Gy         # Enable function level linking
	/utf-8      # Set source and execution charset to utf-8
	/MP         # Build with multiple processes
	/bigobj     # Allow larger .obj files

	/WX-        # Treat warnings as errors
	/permissive-
)
string(REPLACE ";" " " MSVC_COMMON_FLAGS "${MSVC_COMMON_FLAGS}")

set(CMAKE_RC_FLAGS /nologo)

# Override cxx flags
set(CMAKE_CXX_FLAGS "${MSVC_COMMON_FLAGS}" CACHE STRING "C++ Common Flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Zo /Od /Ob0 /Oy- /RTC1 /GS /DDEBUG /D_DEBUG" CACHE STRING "C++ Flags" FORCE)