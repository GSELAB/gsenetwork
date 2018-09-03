SET(runtime_source
    Runtime.cpp)

ADD_SUBDIRECTORY(storage)
ADD_SUBDIRECTORY(vm)
#ADD_SUBDIRECTORY(wasm)

ADD_LIBRARY(runtime ${runtime_source})
TARGET_LINK_LIBRARIES(runtime
    utils runtime_storage database chain core crypto ${GSE_BOOST_LIBS})