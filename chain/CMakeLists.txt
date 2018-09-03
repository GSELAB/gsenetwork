SET (chain_source
     SubChainController.cpp
     Controller.cpp)

ADD_LIBRARY (chain  ${chain_source})
TARGET_LINK_LIBRARIES(chain
    utils crypto database config net ${GSE_BOOST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
