set (producer_source
     ProducerServer.cpp)

add_library (producer STATIC ${producer_source})

target_link_libraries (producer utils)