SET(vm_sources
    Instruction.cpp
    State.cpp
    Memory.cpp
    Stack.cpp
    VMInterface.cpp
    VM.cpp
    Runtime.cpp)

ADD_LIBRARY(vm ${vm_sources})

TARGET_LINK_LIBRARIES(vm
    core jsoncpp_lib_static)