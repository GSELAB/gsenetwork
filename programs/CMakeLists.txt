SET(sources
    main.cpp)

ADD_EXECUTABLE(gsenode ${sources})
TARGET_LINK_LIBRARIES(
    gsenode
    PRIVATE chain core crypto ${GSE_BOOST_LIBS})
