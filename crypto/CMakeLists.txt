SET(crypto_source
     Common.cpp
     Secp256k1PP.cpp
     Encrypt.cpp
     Decrypt.cpp
     SHAx.cpp
     AES.cpp
     SHA3.cpp
     GKey.cpp
     Hash.cpp
     Valid.cpp)

ADD_LIBRARY(crypto ${crypto_source})
TARGET_LINK_LIBRARIES(crypto
    utils core Secp256k1 cryptopp-static ${GSE_BOOST_LIBS})
#cryptopp-static