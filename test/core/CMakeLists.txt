SET(uncry_sources
    UnCry.cpp)

SET(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -w")

ADD_EXECUTABLE(UnCry ${uncry_sources})
TARGET_LINK_LIBRARIES(
    UnCry
    PRIVATE chain core crypto ${GSE_BOOST_LIBS})
