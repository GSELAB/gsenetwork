SET(runtime_security_source
    SEPolicy.cpp
    Security.cpp)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")

ADD_LIBRARY(runtime_security ${runtime_security_source})
TARGET_LINK_LIBRARIES(runtime_security
    utils core ${GSE_BOOST_LIBS})