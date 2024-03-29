function(USE_MSVC_PRECOMPILED_HEADER TargetProject PrecompiledHeader PrecompiledSource)
	if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio")
		# Now only support precompiled headers for the Visual Studio projects
		return()
	endif()

	if (OPTION_PCH AND MSVC)
		message(STATUS "Enable PCH for ${TargetProject}")
		if (WINDOWS)
			if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
				# Inside Visual Studio
				set(PCH_FILE "$(IntDir)$(TargetName).pch")
			else()
				get_filename_component(PCH_NAME "${PrecompiledSource}" NAME_WE)
				set(PCH_FILE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TargetProject}.dir/${PCH_NAME}.pch")
			endif()

			target_compile_options(${TargetProject} PRIVATE "-DUSE_PCH")

			set_source_files_properties("${PrecompiledSource}" PROPERTIES COMPILE_FLAGS " /Yc\"${PrecompiledHeader}\" /Fp\"${PCH_FILE}\" ")
			# Disable Precompiled Header on all C files

			get_target_property(TARGET_SOURCES ${TargetProject} SOURCES)
			foreach(sourcefile ${TARGET_SOURCES})
				if ("${sourcefile}" MATCHES ".*\\.\\cpp$")
				  if (NOT "${sourcefile}" STREQUAL "${PrecompiledSource}")
					set_property(SOURCE "${sourcefile}" APPEND_STRING PROPERTY COMPILE_FLAGS " /Yu\"${PrecompiledHeader}\" /Fp\"${PCH_FILE}\" ")
				  endif()
				endif()
			endforeach(sourcefile)
		endif()
	endif()
endfunction()

macro(add_files)
	list(APPEND SOURCES ${ARGN})
endmacro()

macro(add_to_uberfile uberfile)	
	add_files(${ARGN})
endmacro()

# Automatically add sources in current CMakeLists directory, respecting directory structure
macro(add_sources_recursive_search)	
	set(_src_root_path "${CMAKE_CURRENT_SOURCE_DIR}")

	set(temp ${ARGN})

	if(temp)
		set(_src_cur_path "${CMAKE_CURRENT_SOURCE_DIR}/${ARGN}")
	else()
		set(_src_cur_path "${CMAKE_CURRENT_SOURCE_DIR}")
	endif()

	file(
		GLOB _source_list 
		LIST_DIRECTORIES false
		"${_src_cur_path}/*.cpp"
		"${_src_cur_path}/*.h"
	)

	foreach(_source IN ITEMS ${_source_list})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
		file(RELATIVE_PATH _source_rel "${_src_root_path}" "${_source}")

		if (_group_path STREQUAL "")
			set(_group_path "Root")
		endif()
		
		string(REPLACE "\\" "_" _group_path "${_group_path}")
		
		set(_group_path "${_group_path}.cpp")
		
		list(FIND UBERFILES ${_group_path} GROUP_INDEX)			
		if(GROUP_INDEX EQUAL -1)
			list(APPEND UBERFILES "${_group_path}")
			set(${_group_path}_PROJECTS ${UB_PROJECTS})
		endif()
		
		add_to_uberfile(${_group_path} ${_source_rel})
	endforeach()
endmacro()

macro(read_settings)
	set(options DISABLE_MFC FORCE_STATIC FORCE_SHARED FORCE_SHARED_WIN EDITOR_COMPILE_SETTINGS MSVC_PERMISSIVE)	
	set(oneValueArgs SOLUTION_FOLDER PCH PCH_H OUTDIR)
	set(multiValueArgs FILE_LIST INCLUDES LIBS DEFINES)
	cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
endmacro()

macro(prepare_project)
	set(THIS_PROJECT ${target} PARENT_SCOPE)
	set(THIS_PROJECT ${target})
	include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" )
	project(${target})

	message(STATUS "prepare project THIS_PROJECT = ${THIS_PROJECT}")
	
	if(NOT ${THIS_PROJECT}_SOURCES)
		set(${THIS_PROJECT}_SOURCES ${SOURCES})
	endif()
endmacro()

macro(apply_compile_settings)
	if (MODULE_PCH)
		USE_MSVC_PRECOMPILED_HEADER( ${THIS_PROJECT} ${MODULE_PCH_H} ${MODULE_PCH} )
		set_property(TARGET ${THIS_PROJECT} APPEND PROPERTY AUTOMOC_MOC_OPTIONS -b${MODULE_PCH_H})
	endif()

	option(ASAN_ENABLED "Build this target with AddressSanitizer" OFF)

	if(ASAN_ENABLED)
		if(MSVC)
			target_compile_options(${THIS_PROJECT} PUBLIC /fsanitize=address)
		else()
			target_compile_options(${THIS_PROJECT} PUBLIC -fsanitize=address)
			target_link_options(${THIS_PROJECT} PUBLIC -fsanitize=address)
		endif()
	endif()

	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		target_compile_definitions(${THIS_PROJECT} PRIVATE "DEBUG")
	endif()
endmacro()

function(CryEngineModule target)
	prepare_project(${ARGN})	
endfunction()

macro(prepare_project_after)
	message(STATUS "THIS_PROJECT = ${THIS_PROJECT}")
	message(STATUS "THIS_PROJECT_SOURCES = ${${THIS_PROJECT}_SOURCES}")
	message(STATUS "MODULE_FORCE_SHARED = ${MODULE_FORCE_SHARED}")
	message(STATUS "MODULE_FORCE_STATIC = ${MODULE_FORCE_STATIC}")
	message(STATUS "BUILD_SHARED_LIBS = ${BUILD_SHARED_LIBS}")

	read_settings(${ARGN})

	apply_compile_settings()

	add_gsl_lib()

	get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
	foreach(dir ${dirs})
  		message(STATUS "dir='${dir}'")
	endforeach()
endmacro()

function(CryConsoleApplication target)
	prepare_project(${ARGN})
	add_executable(${THIS_PROJECT} ${${THIS_PROJECT}_SOURCES})
	if(WINDOWS AND MSVC)
		set_property(TARGET ${THIS_PROJECT} APPEND_STRING PROPERTY LINK_FLAGS " /SUBSYSTEM:CONSOLE")
	endif()
	apply_compile_settings()	
endfunction()

function(CryFileContainer cur_folder)
	add_sources_recursive_search(${cur_folder})

	set(${THIS_PROJECT}_SOURCES ${${THIS_PROJECT}_SOURCES} ${SOURCES} PARENT_SCOPE)
endfunction()

macro(add_gsl_lib)
	include(FetchContent)
		FetchContent_Declare(GSL
    	GIT_REPOSITORY "https://github.com/microsoft/GSL"
    	GIT_TAG "v3.1.0"
	)
	FetchContent_MakeAvailable(GSL)

	target_link_libraries(${THIS_PROJECT} GSL)

	# get_target_property(MS_GSL_INCLUDE_DIR GSL INTERFACE_INCLUDE_DIRECTORIES)
	# include_directories( "${MS_GSL_INCLUDE_DIR}" )
	# message(STATUS "MS_GSL_INCLUDE_DIR = ${MS_GSL_INCLUDE_DIR}")
endmacro()