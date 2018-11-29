SET(runtime_common_source
    GVMThread.cpp
    Runtime.cpp)

ADD_LIBRARY(runtime_common ${runtime_common_source})
TARGET_LINK_LIBRARIES(runtime_common
    utils core ${GSE_BOOST_LIBS})