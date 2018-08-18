set (rpc_source
     RpcService.cpp)

add_library (rpc STATIC ${rpc_source})

target_link_libraries (rpc utils)