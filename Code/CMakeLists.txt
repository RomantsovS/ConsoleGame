if (OPTION_ENGINE)
	set(OPTION_LAUNCHER_EXECUTABLE "GameLauncher" CACHE STRING "Base file name for the game executable (without file extension)")

	set(THIS_PROJECT "ConsoleGame")

	include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )
	include_directories( "${CMAKE_SOURCE_DIR}" )

	project(${THIS_PROJECT})
	read_settings(PCH "framework/precompiled.cpp" PCH_H "idlib/precompiled.h")
	
	CryFileContainer("cm")
	CryFileContainer("d3xp")
	CryFileContainer("framework")
	CryFileContainer("renderer")
	CryFileContainer("SWF")
	CryFileContainer("sys")

	message(STATUS "THIS_PROJECT = ${THIS_PROJECT}")
	message(STATUS "THIS_PROJECT_SOURCES = ${${THIS_PROJECT}_SOURCES}")

	if(WINDOWS)
		message(STATUS "add_executable THIS_PROJECT = ${THIS_PROJECT}")
		add_executable(${THIS_PROJECT} WIN32 ${${THIS_PROJECT}_SOURCES})
	else()
		add_executable(${target} ${${THIS_PROJECT}_SOURCES})
	endif()
	if(NOT ANDROID)
		set_property(TARGET ${THIS_PROJECT} PROPERTY OUTPUT_NAME "${OPTION_LAUNCHER_EXECUTABLE}")	
	endif()

	if(WINDOWS)
		set_property(TARGET ${THIS_PROJECT} APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
	endif()

	target_link_libraries(${THIS_PROJECT} idlib)

	apply_compile_settings()
endif()