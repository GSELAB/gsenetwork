SET(sources
    main.cpp)

SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -w")

ADD_EXECUTABLE(gsenode ${sources})
TARGET_LINK_LIBRARIES(
    gsenode
    PRIVATE chain core crypto ${GSE_BOOST_LIBS})
