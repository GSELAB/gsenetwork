cmake_minimum_required(VERSION 2.8.12)

project (GSENetwork)

enable_language(CXX)
enable_language(C)
enable_language(ASM)


include_directories (
    "${PROJECT_SOURCE_DIR}/bundle/include"
    "${PROJECT_SOURCE_DIR}/chain/include"
    "${PROJECT_SOURCE_DIR}/config/include"
    "${PROJECT_SOURCE_DIR}/core/include"
    "${PROJECT_SOURCE_DIR}/database/include"
    "${PROJECT_SOURCE_DIR}/net/include"
    "${PROJECT_SOURCE_DIR}/producer/include"
    "${PROJECT_SOURCE_DIR}/rpc/include"
    "${PROJECT_SOURCE_DIR}/proto"
    "${PROJECT_SOURCE_DIR}/utils/include")

#
add_subdirectory (proto)
add_subdirectory (utils)
add_subdirectory (net)
add_subdirectory (database)
add_subdirectory (config)
add_subdirectory (core)
add_subdirectory (runtime)
add_subdirectory (bundle)
add_subdirectory (rpc)
add_subdirectory (producer)
add_subdirectory (chain)
add_subdirectory (programs)