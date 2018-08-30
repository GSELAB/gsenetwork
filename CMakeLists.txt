CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

INCLUDE(cmake/cable/bootstrap.cmake)
INCLUDE(CableBuildInfo)
INCLUDE(CableBuildType)
INCLUDE(CableToolchains)
INCLUDE(GNUInstallDirs)

IF(UNIX)
    MESSAGE(STATUS "Unix system")
    SET(toolchain cxx11-pic)
ELSE()
    SET(toolchain cxx11)
ENDIF()
cable_configure_toolchain(DEFAULT ${toolchain})

SET(GSE_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/cmake" CACHE PATH "The path to the cmake directory")
LIST(APPEND CMAKE_MODULE_PATH ${GSE_CMAKE_DIR})

MESSAGE(STATUS "CMAKE_CXX_STANDARD:${CMAKE_CXX_STANDARD}")
MESSAGE(STATUS "CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")


# Map current configuration to configurations of imported targets.
SET(CMAKE_MAP_IMPORTED_CONFIG_DEBUG Release)
SET(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)

SET(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Hunter configuration types")
SET(HUNTER_JOBS_NUMBER 4 CACHE STRING "Hunter jobs number")
SET(HUNTER_CACHE_SERVERS "https://github.com/ethereum/hunter-cache" CACHE STRING "Hunter cache servers")
SET(HUNTER_PASSWORDS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Hunter/passwords.cmake CACHE STRING "Hunter password path")

# In CI builds upload the binaries if the GITHUB_USER_PASSWORD was decrypted
# (only for branches and internal PRs).
IF("$ENV{CI}" AND NOT "$ENV{GITHUB_USER_PASSWORD}" STREQUAL "")
    SET(run_upload YES)
ELSE()
    SET(run_upload NO)
ENDIF()
OPTION(HUNTER_RUN_UPLOAD "Upload binaries to the cache server" ${run_upload})

INCLUDE(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.21.16.tar.gz"
    SHA1 "aa1faccd24f33dd2c368f85a4e9ca2e5d9c249cb"
    LOCAL
)

# *************************************** GSE Network ******************************************** #
PROJECT(GSENetwork)

SET(PROJECT_VERSION 1.0)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug;Release;RelWithDebInfo)

cable_add_buildinfo_library(PROJECT_NAME gsenode)

SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY On)

INCLUDE(GSECcache)
INCLUDE(GSEOptions)
INCLUDE(GSECompilerSettings)
INCLUDE(GSEExecutableHelper)
INCLUDE(GSEDependencies)
INCLUDE(GSEUtils)

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
hunter_add_package(Boost COMPONENTS program_options filesystem system thread context fiber log)
find_package(Boost CONFIG REQUIRED program_options filesystem system thread context fiber log)

SET(GSE_BOOST_LIBS
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_ATOMIC_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    )
MESSAGE(STATUS "GSE_BOOST_LIBS:----> ${GSE_BOOST_LIBS}")

hunter_add_package(cryptopp)
find_package(cryptopp CONFIG REQUIRED)

#hunter_add_package(libscrypt)
#find_package(libscrypt CONFIG REQUIRED)

find_package(Threads)

MESSAGE(STATUS "MINIUPNPC:${MINIUPNPC}")
IF(MINIUPNPC)
    find_package(Miniupnpc 1.8.2013 REQUIRED)
ENDIF()

SET(UTILS_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/utils/include")

configure_project()

# no test module exist util now. (not for the future)

MESSAGE(STATUS "Boost_INCLUDE_DIR:${Boost_INCLUDE_DIR}")
include_directories (
    ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/database/include/leveldb/include"
    "${PROJECT_SOURCE_DIR}/core/include"
    "${PROJECT_SOURCE_DIR}/crypto/include"
    "${PROJECT_SOURCE_DIR}/chain/include"
    "${PROJECT_SOURCE_DIR}/config/include"
    "${PROJECT_SOURCE_DIR}/database/include"
    "${PROJECT_SOURCE_DIR}/net/include"
    "${PROJECT_SOURCE_DIR}/producer/include"
    "${PROJECT_SOURCE_DIR}/rpc/include"
    "${PROJECT_SOURCE_DIR}/utils/include")

#link_directories(
#    "${PROJECT_SOURCE_DIR}/cmake/install/boost_1_63_0/stage/lib")


add_subdirectory (config)
add_subdirectory (utils)
add_subdirectory (core)

add_subdirectory (crypto)
add_subdirectory (database)

add_subdirectory (rpc)
add_subdirectory (net)

add_subdirectory (runtime)
add_subdirectory (producer)
add_subdirectory (chain)

add_subdirectory (programs)