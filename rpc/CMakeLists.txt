set (rpc_source
     RpcService.cpp)

#add_library (rpc STATIC ${rpc_source})

add_library (rpc ${rpc_source})

target_link_libraries (rpc utils)