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

# ----------------------- GSE Network Project ----------------------- #
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

# Boost
SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
hunter_add_package(Boost COMPONENTS program_options filesystem system thread context fiber log)
FIND_PACKAGE(Boost CONFIG REQUIRED program_options filesystem system thread context fiber log)
SET(GSE_BOOST_LIBS
    ${Boost_LOG_LIBRARY}
    ${Boost_LOG_SETUP_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${Boost_ATOMIC_LIBRARY}
    ${Boost_CONTEXT_LIBRARY}
    )

hunter_add_package(cryptopp)
FIND_PACKAGE(cryptopp CONFIG REQUIRED)

#hunter_add_package(libscrypt)
#FIND_PACKAGE(libscrypt CONFIG REQUIRED)

FIND_PACKAGE(Threads)


IF(MINIUPNPC)
    MESSAGE(STATUS "MINIUPNPC is configured")
    FIND_PACKAGE(Miniupnpc 1.8.2013 REQUIRED)
ENDIF()

CONFIGURE_PROJECT()
INCLUDE_DIRECTORIES(
#   ${CMAKE_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
    "${PROJECT_SOURCE_DIR}/database/include/leveldb/include"
    "${PROJECT_SOURCE_DIR}/core/include"
    "${PROJECT_SOURCE_DIR}/runtime/storage/include"
    "${PROJECT_SOURCE_DIR}/runtime/vm/include"
    "${PROJECT_SOURCE_DIR}/runtime/include"
    "${PROJECT_SOURCE_DIR}/crypto/include"
    "${PROJECT_SOURCE_DIR}/chain/include"
    "${PROJECT_SOURCE_DIR}/config/include"
    "${PROJECT_SOURCE_DIR}/database/include"
    "${PROJECT_SOURCE_DIR}/net/include"
    "${PROJECT_SOURCE_DIR}/producer/include"
    "${PROJECT_SOURCE_DIR}/rpc/include"
    "${PROJECT_SOURCE_DIR}/utils/include")

ADD_SUBDIRECTORY(config)
ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(crypto)
ADD_SUBDIRECTORY(database)
ADD_SUBDIRECTORY(rpc)
ADD_SUBDIRECTORY(net)
ADD_SUBDIRECTORY(runtime)
ADD_SUBDIRECTORY(producer)
ADD_SUBDIRECTORY(chain)

ADD_SUBDIRECTORY(programs)