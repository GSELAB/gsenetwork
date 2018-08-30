set (crypto_source
     Common.cpp
     SHA3.cpp
     Hash.cpp)

add_library (crypto ${crypto_source})

target_link_libraries (crypto utils ${GSE_BOOST_LIBS})