IF(MSVC)
	IF(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0.0)
		MESSAGE(FATAL_ERROR "ERROR - As of the 1.3.0 release, this project only supports Visual Studio 2015 or newer.\nPlease download from https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx.")
	ELSE()
		get_filename_component(GSE_DEPENDENCY_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../deps/x64" ABSOLUTE)
	ENDIF()
	SET(CMAKE_PREFIX_PATH ${GSE_DEPENDENCY_INSTALL_DIR} ${CMAKE_PREFIX_PATH})
ENDIF()

# custom cmake scripts
SET(GSE_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR})
SET(GSE_SCRIPTS_DIR ${GSE_CMAKE_DIR}/scripts)

