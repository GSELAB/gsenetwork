SET(utils_source
    Utils.cpp)

ADD_LIBRARY(utils ${utils_source})

TARGET_LINK_LIBRARIES(utils
    ${GSE_BOOST_LIBS})