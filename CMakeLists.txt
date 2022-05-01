cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED CRYENGINE_DIR)
	set(CRYENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

include(Tools/CMake/InitialSetup.cmake)

# Setting the project version
if (NOT VERSION)
	if (METADATA_VERSION)
		set(VERSION ${METADATA_VERSION})
	else()
		set(VERSION "1.0.0.0")
	endif()
endif()
set(METADATA_VERSION ${VERSION} CACHE STRING "Version number for executable metadata" FORCE)

# Declare the project.
set(METADATA_PROJECT_NAME "ConsoleGame" CACHE STRING "Name of the solution project")
project("${METADATA_PROJECT_NAME}_${BUILD_PLATFORM}"
		VERSION "${METADATA_VERSION}"
		LANGUAGES C CXX)

include(Tools/CMake/Configure.cmake)