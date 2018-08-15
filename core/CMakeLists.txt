set (core_source
     *.cpp)

add_library (core STATIC ${core_source})

target_link_libraries (net utils bundle)