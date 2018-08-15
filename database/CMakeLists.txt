
set (storage_source
	*.cpp)

add_subdirectory (leveldb)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/leveldb/include")

add_library (database STATIC ${storage_source})

target_link_libraries (database leveldb)
