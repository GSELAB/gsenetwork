set (config_source
     DefaultConfig.cpp)

add_library (config STATIC ${config_source})

#target_link_libraries (config utils bundle)