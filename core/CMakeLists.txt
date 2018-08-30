SET(core_source
    FixedHash.cpp
    CommonData.cpp
    Common.cpp
    Log.cpp
    RLP.cpp
    Address.cpp
    Worker.cpp)

ADD_LIBRARY(core ${core_source})

target_link_libraries(core
    utils ${GSE_BOOST_LIBS})