SET(listener_source
    EventObserve.cpp)

ADD_LIBRARY(listener ${listener_source})
TARGET_LINK_LIBRARIES(listener
    core utils ${GSE_BOOST_LIBS})