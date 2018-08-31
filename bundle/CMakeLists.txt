SET(bundle_source
     AccountBundle.cpp
     TransactionBundle.cpp
     BlockBundle.cpp
     ProducerBundle.cpp
     SubChainBundle.cpp)

ADD_LIBRARY(bundle ${bundle_source})
TARGET_LINK_LIBRARIES(bundle
    utils)