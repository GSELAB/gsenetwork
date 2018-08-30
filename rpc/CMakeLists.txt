SET(rpc_source
    RpcService.cpp)

ADD_LIBRARY(rpc ${rpc_source})
TARGET_LINK_LIBRARIES(rpc
    utils ${GSE_BOOST_LIBS})