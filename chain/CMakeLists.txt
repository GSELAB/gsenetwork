SET (chain_source
    Common.cpp
    Genesis.cpp
    BlockState.cpp
    RollbackState.cpp
    SubChainController.cpp
    BlockChain.cpp
    Sync.cpp
    Controller.cpp)

ADD_LIBRARY (chain  ${chain_source})
TARGET_LINK_LIBRARIES(chain
    utils core crypto database config net producer runtime rpc ${GSE_BOOST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
