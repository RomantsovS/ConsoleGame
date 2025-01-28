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
	if(WINDOWS)
	CryFileContainer("sys/win32")
	else()
	CryFileContainer("sys/linux")
	endif()

	message(STATUS "THIS_PROJECT = ${THIS_PROJECT}")
	message(STATUS "THIS_PROJECT_SOURCES = ${${THIS_PROJECT}_SOURCES}")

	message(STATUS "add_executable THIS_PROJECT = ${THIS_PROJECT}")

	find_package(Boost REQUIRED)
	find_package(Threads)

	find_package(Protobuf CONFIG REQUIRED)
	if (NOT Protobuf_FOUND)
		find_package(Protobuf REQUIRED)
	endif()
	file(GLOB _proto_source_list LIST_DIRECTORIES false "${PROJECT_DIR}/Proto/*.proto")

	if(WINDOWS)
		add_executable(${THIS_PROJECT} WIN32 ${${THIS_PROJECT}_SOURCES} ${_proto_source_list})
	else()
		add_executable(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES} ${PROTO_SRCS})
	endif()
	if(NOT ANDROID)
		set_property(TARGET ${THIS_PROJECT} PROPERTY OUTPUT_NAME "${OPTION_LAUNCHER_EXECUTABLE}")	
	endif()

	message(STATUS "CMAKE_CURRENT_LIST_DIR=${CMAKE_CURRENT_LIST_DIR}")

	#set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
	#target_include_directories(proto-objects PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
	protobuf_generate(
		TARGET ${THIS_PROJECT}
		IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../proto"
		#PROTOC_OUT_DIR "${PROTO_BINARY_DIR}"
	)

	if(WINDOWS AND MSVC)
		set_property(TARGET ${THIS_PROJECT} APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
	endif()

	target_include_directories(${THIS_PROJECT} PRIVATE ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	target_link_libraries(${THIS_PROJECT} ${CMAKE_THREAD_LIBS_INIT} protobuf::libprotobuf idlib)

	prepare_project_after(PCH "framework/precompiled.cpp" PCH_H "idlib/precompiled.h")
endif()