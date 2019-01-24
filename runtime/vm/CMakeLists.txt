SET(vm_sources
    Instruction.cpp)

ADD_LIBRARY(vm ${vm_sources})

TARGET_LINK_LIBRARIES(vm
    core jsoncpp_lib_static)