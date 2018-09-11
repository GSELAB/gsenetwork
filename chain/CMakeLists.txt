SET (chain_source
    Genesis.cpp
    SubChainController.cpp
    BlockChain.cpp
    Controller.cpp)

ADD_LIBRARY (chain  ${chain_source})
TARGET_LINK_LIBRARIES(chain
    utils core crypto database config net ${GSE_BOOST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
