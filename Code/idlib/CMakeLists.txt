CryEngineModule(idlib)

CryFileContainer("")
CryFileContainer("bv")
CryFileContainer("containers")
CryFileContainer("geometry")
CryFileContainer("math")
CryFileContainer("sys")

if (MODULE_FORCE_SHARED)
	add_library(${THIS_PROJECT} SHARED ${${THIS_PROJECT}_SOURCES})
elseif (MODULE_FORCE_STATIC)
	add_library(${THIS_PROJECT} STATIC ${${THIS_PROJECT}_SOURCES})
else()
	add_library(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
endif()

prepare_project_after(PCH "precompiled.cpp" PCH_H "precompiled.h")