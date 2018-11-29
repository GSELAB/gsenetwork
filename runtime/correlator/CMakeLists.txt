SET(runtime_correlator_source
    Correlation.cpp)

ADD_LIBRARY(runtime ${runtime_correlator_source})

TARGET_LINK_LIBRARIES(runtime
    utils runtime_common runtime_storage runtime_action database chain core crypto ${GSE_BOOST_LIBS})