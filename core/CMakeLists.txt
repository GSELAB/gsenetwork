set (core_source
     FixedHash.cpp
     CommonData.cpp
     Common.cpp
     Log.cpp
     RLP.cpp
     Address.cpp
     Worker.cpp)

#add_library (core STATIC ${core_source})

add_library (core ${core_source})

# target_link_libraries (net utils bundle)