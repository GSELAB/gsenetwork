cmake_minimum_required(VERSION 3.5)

project (GSENetwork)

set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_EXTENSIONS ON )
set( CXX_STANDARD_REQUIRED ON)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 2)
set(VERSION_PATCH 1)


#enable_language(CXX)
#enable_language(CC)
#enable_language(C)
#enable_language(ASM)


## find boost library..
#find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Boost COMPONENTS)
if(Boost_FOUND)
    message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
    message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
    message(status "** Boost Libraries: ${Boost_LIBRARIES}")
    include_directories(${Boost_INCLUDE_DIR})
    link_directories(${Boost_LIBRARY_DIR})
else()
    message(status "Boost not installed, Download it may cost more than 5 minute......")
    # install boost
    include ("${CMAKE_CURRENT_LIST_DIR}/cmake/install_boost.cmake")
endif()

## install secp256k1
include ("${CMAKE_CURRENT_LIST_DIR}/cmake/install_secp256k1.cmake")


include_directories (
    "${PROJECT_SOURCE_DIR}/database/include/leveldb/include"
    "${PROJECT_SOURCE_DIR}/core/include"
    "${PROJECT_SOURCE_DIR}/crypto/include"
    "${PROJECT_SOURCE_DIR}/bundle/include"
    "${PROJECT_SOURCE_DIR}/chain/include"
    "${PROJECT_SOURCE_DIR}/config/include"
    "${PROJECT_SOURCE_DIR}/database/include"
    "${PROJECT_SOURCE_DIR}/net/include"
    "${PROJECT_SOURCE_DIR}/producer/include"
    "${PROJECT_SOURCE_DIR}/rpc/include"
    "${PROJECT_SOURCE_DIR}/proto"
    "${PROJECT_SOURCE_DIR}/utils/include"
    "${PROJECT_SOURCE_DIR}/cmake/install/boost_1_61_0")

link_directories(
    "${PROJECT_SOURCE_DIR}/cmake/install/boost_1_61_0/stage/lib")


add_subdirectory (proto)
add_subdirectory (utils)
add_subdirectory (core)
add_subdirectory (crypto)
add_subdirectory (net)
add_subdirectory (database)
add_subdirectory (config)
add_subdirectory (runtime)
add_subdirectory (bundle)
add_subdirectory (rpc)
add_subdirectory (producer)
add_subdirectory (chain)
add_subdirectory (programs)