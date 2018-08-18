
include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/leveldb/include")
add_subdirectory (leveldb)

set (database_source
	Database.cpp
	ProducerStore.cpp
	SubChainStore.cpp)

add_library (database STATIC ${database_source})

target_link_libraries (database leveldb)
