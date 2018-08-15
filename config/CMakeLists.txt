set (config_source
     *.cpp)

add_library (config STATIC ${config_source})

target_link_libraries (config utils bundle)