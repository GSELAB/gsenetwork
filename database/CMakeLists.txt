
set (storage_source
	database.c)

add_subdirectory (leveldb)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/leveldb/include")

add_library (storage STATIC ${storage_source})

target_link_libraries (storage leveldb)
