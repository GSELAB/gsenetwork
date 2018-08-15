set (bundle_source
     *.cpp)

add_library (bundle STATIC ${bundle_source})

target_link_libraries (bundle utils)