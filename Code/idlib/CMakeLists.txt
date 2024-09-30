CryEngineModule(idlib)

CryFileContainer("")
CryFileContainer("bv")
CryFileContainer("containers")
CryFileContainer("geometry")
CryFileContainer("math")
CryFileContainer("sys")

find_package(Boost REQUIRED)
find_package(Threads)

find_package(Protobuf REQUIRED) # use without CONFIG to call cmake's FindProtobuf to define Protobuf_INCLUDE_DIRS
message(STATUS "Protobuf_INCLUDE_DIRS=${Protobuf_INCLUDE_DIRS}")

if (MODULE_FORCE_SHARED)
	add_library(${THIS_PROJECT} SHARED ${${THIS_PROJECT}_SOURCES})
elseif (MODULE_FORCE_STATIC)
	add_library(${THIS_PROJECT} STATIC ${${THIS_PROJECT}_SOURCES})
else()
	add_library(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
endif()

target_include_directories(${THIS_PROJECT} PRIVATE ${Boost_INCLUDE_DIR} ${Protobuf_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

prepare_project_after(PCH "precompiled.cpp" PCH_H "precompiled.h")