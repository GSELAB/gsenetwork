SET(runtime_storage_source
    Repository.cpp)

ADD_LIBRARY(runtime_storage ${runtime_storage_source})
TARGET_LINK_LIBRARIES(runtime_storage
    utils database core ${GSE_BOOST_LIBS})