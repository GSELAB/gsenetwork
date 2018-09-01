# Copyright (c) 2018 GSENetwork
# This file is part of GSENetwork.

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/leveldb/include")
ADD_SUBDIRECTORY(leveldb)

SET(database_source
	Database.cpp
    DatabaseController.cpp
    )

ADD_LIBRARY(database ${database_source})
TARGET_LINK_LIBRARIES(database
    leveldb)
