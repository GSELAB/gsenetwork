set (net_source
     Common.cpp
     Capability.cpp
     Host.cpp
     HostCapability.cpp
     Network.cpp
     NodeTable.cpp
     Peer.cpp
     RLPXFrameCoder.cpp
     RLPxHandshake.cpp
     Session.cpp
     UDP.cpp
     UPnp.cpp)

add_library (net STATIC ${net_source})

target_link_libraries (net config)