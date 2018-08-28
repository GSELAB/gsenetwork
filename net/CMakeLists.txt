set (net_source
    BytesSocket.cpp
    BytesPacket.cpp
    BytesFrameCoder.cpp
    BytesHandshake.cpp
    Common.cpp
    Network.cpp
    UDP.cpp
    UPnp.cpp
    Peer.cpp
    RLPXFrameCoder.cpp
    RLPxHandshake.cpp
    NodeTable.cpp
    Capability.cpp
    Session.cpp
    Host.cpp)

add_library (net ${net_source})

#add_library (net STATIC ${net_source})

target_link_libraries (net core crypto config bundle)

#file(GLOB net_sources "*.cpp")

#add_library(net ${net_sources})
#target_link_libraries(net core crypto config bundle)
#target_include_directories(net SYSTEM PRIVATE)