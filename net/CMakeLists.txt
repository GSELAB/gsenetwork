set (net_source
     *.cpp)

add_library (net STATIC ${net_source})

target_link_libraries (net config)