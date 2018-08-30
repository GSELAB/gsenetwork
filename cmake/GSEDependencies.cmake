
if (MSVC)
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0)
		message(FATAL_ERROR "ERROR - As of the 1.3.0 release, this project only supports Visual Studio 2015 or newer.\nPlease download from https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx.")
	else()
		get_filename_component(GSE_DEPENDENCY_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../deps/x64" ABSOLUTE)
	endif()
	set (CMAKE_PREFIX_PATH ${GSE_DEPENDENCY_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
endif()

# custom cmake scripts
set(GSE_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(GSE_SCRIPTS_DIR ${GSE_CMAKE_DIR}/scripts)

