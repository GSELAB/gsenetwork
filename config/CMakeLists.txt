SET(config_source
    DefaultConfig.cpp)

ADD_LIBRARY(config ${config_source})
TARGET_LINK_LIBRARIES(config
    utils ${GSE_BOOST_LIBS})