SET(config_source
    Constant.cpp
    DefaultConfig.cpp
    Argument.cpp)

ADD_LIBRARY(config ${config_source})
TARGET_LINK_LIBRARIES(config
    utils chain ${GSE_BOOST_LIBS})