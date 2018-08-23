set (chain_source
     DbManager.cpp
     NetController.cpp
     SubChainController.cpp)

#add_library (chain STATIC ${chain_source})
add_library (chain ${chain_source})
target_link_libraries (chain utils bundle crypto database config net)