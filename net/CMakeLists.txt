SET(net_source
    BytesSocket.cpp
    BytesPacket.cpp
    BytesFrameCoder.cpp
    BytesHandshake.cpp
    UDPx.cpp
    NodeTablex.cpp
    Common.cpp
    Network.cpp
    UPnp.cpp
    PeerCapability.cpp
    Peer.cpp
    Session.cpp
    GSEPeer.cpp
    Client.cpp
    Host.cpp
    NetController.cpp)

ADD_LIBRARY(net ${net_source})
TARGET_LINK_LIBRARIES(net
    core crypto config ${GSE_BOOST_LIBS})