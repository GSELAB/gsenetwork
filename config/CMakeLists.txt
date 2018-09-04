SET(config_source
    Constant.cpp
    DefaultConfig.cpp)

ADD_LIBRARY(config ${config_source})
TARGET_LINK_LIBRARIES(config
    utils ${GSE_BOOST_LIBS})