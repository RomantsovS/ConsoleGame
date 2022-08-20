if (OPTION_ENGINE)
	set(OPTION_LAUNCHER_EXECUTABLE "GameLauncher" CACHE STRING "Base file name for the game executable (without file extension)")

	CryEngineModule(ConsoleGame)
	
	CryFileContainer("cm")
	CryFileContainer("d3xp")
	CryFileContainer("d3xp/ai")
	CryFileContainer("d3xp/anim")
	CryFileContainer("d3xp/gamesys")
	CryFileContainer("d3xp/menus")
	CryFileContainer("d3xp/physics")
	CryFileContainer("framework")
	CryFileContainer("renderer")
	CryFileContainer("SWF")
	CryFileContainer("sys")
	CryFileContainer("sys/win32")

	message(STATUS "THIS_PROJECT = ${THIS_PROJECT}")
	message(STATUS "THIS_PROJECT_SOURCES = ${${THIS_PROJECT}_SOURCES}")

	message(STATUS "add_executable THIS_PROJECT = ${THIS_PROJECT}")
	
	if(WINDOWS)
		add_executable(${THIS_PROJECT} WIN32 ${${THIS_PROJECT}_SOURCES})
	else()
		add_executable(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
	endif()
	if(NOT ANDROID)
		set_property(TARGET ${THIS_PROJECT} PROPERTY OUTPUT_NAME "${OPTION_LAUNCHER_EXECUTABLE}")	
	endif()

	if(WINDOWS AND MSVC)
		set_property(TARGET ${THIS_PROJECT} APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
	endif()

	target_link_libraries(${THIS_PROJECT} idlib)

	prepare_project_after(PCH "framework/precompiled.cpp" PCH_H "idlib/precompiled.h")
endif()