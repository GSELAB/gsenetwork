SET(crypto_source
     Common.cpp
     SHA3.cpp
     Hash.cpp)

ADD_LIBRARY(crypto ${crypto_source})
TARGET_LINK_LIBRARIES(crypto
    utils core ${GSE_BOOST_LIBS})