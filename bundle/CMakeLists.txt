set (bundle_source
     AccountBundle.cpp
     TransactionBundle.cpp
     BlockBundle.cpp
     ProducerBundle.cpp
     SubChainBundle.cpp)

#add_library (bundle STATIC ${bundle_source})
add_library (bundle ${bundle_source})
target_link_libraries (bundle utils)