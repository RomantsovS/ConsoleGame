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

	find_package(Boost COMPONENTS system REQUIRED)
	find_package(Threads)

	find_package(Protobuf REQUIRED)
	
	file(GLOB _proto_source_list LIST_DIRECTORIES false "${PROJECT_DIR}/Proto/*.proto")

	protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${_proto_source_list})

	if(WINDOWS)
		add_executable(${THIS_PROJECT} WIN32 ${${THIS_PROJECT}_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
	else()
		add_executable(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES} ${PROTO_SRCS} ${PROTO_HDRS})
	endif()
	if(NOT ANDROID)
		set_property(TARGET ${THIS_PROJECT} PROPERTY OUTPUT_NAME "${OPTION_LAUNCHER_EXECUTABLE}")	
	endif()

	if(WINDOWS AND MSVC)
		set_property(TARGET ${THIS_PROJECT} APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
	endif()

	target_include_directories(${THIS_PROJECT} PRIVATE ${Boost_INCLUDE_DIR} ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${THIS_PROJECT} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Protobuf_LIBRARIES} idlib)

	prepare_project_after(PCH "framework/precompiled.cpp" PCH_H "idlib/precompiled.h")
endif()