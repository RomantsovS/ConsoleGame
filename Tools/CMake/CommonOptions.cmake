if (NOT PROJECT_BUILD)
	option(OPTION_ENGINE "Build CRYENGINE" ON)
else()
	option(OPTION_ENGINE "Build CRYENGINE" OFF)
endif()

option(OPTION_PCH "Enable Precompiled Headers" ON)