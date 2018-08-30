SET(producer_source
    Producer.cpp
    ProducerServer.cpp)

ADD_LIBRARY(producer ${producer_source})
TARGET_LINK_LIBRARIES(producer
    chain utils ${GSE_BOOST_LIBS})