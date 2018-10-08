SET(runtime_action_source
    Transfer.cpp
    Vote.cpp
    BeenProducer.cpp
    ActionFactory.cpp)

ADD_LIBRARY(runtime_action ${runtime_action_source})
TARGET_LINK_LIBRARIES(runtime_action
    utils core ${GSE_BOOST_LIBS})