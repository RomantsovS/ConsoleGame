CryEngineModule(idlib)

CryFileContainer("")
CryFileContainer("bv")
CryFileContainer("containers")
CryFileContainer("geometry")
CryFileContainer("math")
CryFileContainer("sys")

find_package(Boost REQUIRED)
find_package(Threads)

find_package(Protobuf CONFIG REQUIRED)
if (NOT Protobuf_FOUND)
	find_package(Protobuf REQUIRED)
endif()

if (MODULE_FORCE_SHARED)
	add_library(${THIS_PROJECT} SHARED ${${THIS_PROJECT}_SOURCES})
elseif (MODULE_FORCE_STATIC)
	add_library(${THIS_PROJECT} STATIC ${${THIS_PROJECT}_SOURCES})
else()
	add_library(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
endif()

target_include_directories(${THIS_PROJECT} PRIVATE ${Boost_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${THIS_PROJECT} protobuf::libprotobuf)

prepare_project_after(PCH "precompiled.cpp" PCH_H "precompiled.h")