set (crypto_source
     Common.cpp)

#add_library (crypto STATIC ${crypto_source})

add_library (crypto ${crypto_source})

# target_link_libraries (crypto utils bundle)