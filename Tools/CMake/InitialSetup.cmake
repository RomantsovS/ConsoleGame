if (NOT DEFINED CRYENGINE_DIR)
    message(FATAL_ERROR "CRYENGINE_DIR is not set. Please set it before including InitialSetup.cmake")
endif()

# Set the PROJECT_DIR to the source path used to start CMake
if(NOT DEFINED PROJECT_DIR)
	set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
endif()

# Fix slashes on paths
file(TO_CMAKE_PATH "${CRYENGINE_DIR}" CRYENGINE_DIR)
file(TO_CMAKE_PATH "${PROJECT_DIR}" PROJECT_DIR)
file(TO_CMAKE_PATH "${TOOLS_CMAKE_DIR}" TOOLS_CMAKE_DIR)

set(TOOLS_CMAKE_DIR "${CRYENGINE_DIR}/Tools/CMake")

message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
message(STATUS "OPTION_PCH = ${OPTION_PCH}")
message(STATUS "CRYENGINE_DIR = ${CRYENGINE_DIR}")
message(STATUS "PROJECT_DIR = ${PROJECT_DIR}")
message(STATUS "TOOLS_CMAKE_DIR = ${TOOLS_CMAKE_DIR}")

message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
# Including the Toolchain file, as it sets important variables.
if(DEFINED CMAKE_TOOLCHAIN_FILE)
	#include(${CMAKE_TOOLCHAIN_FILE})
endif()

if(WIN32)
	if(CMAKE_GENERATOR MATCHES "^Visual Studio.*")
		include("${TOOLS_CMAKE_DIR}/toolchain/windows/WindowsPC-MSVC.cmake")
	else()
		include("${TOOLS_CMAKE_DIR}/WindowsPC-GCC.cmake")
	endif()
else()
	include("${TOOLS_CMAKE_DIR}/Linux-GCC.cmake")
endif()

if (NOT DEFINED BUILD_PLATFORM)
	# For now, we expect BUILD_PLATFORM to have been set via a Toolchain file.
	message(FATAL_ERROR "BUILD_PLATFORM not defined. Please always supply one of the CRYENGINE toolchain files.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)