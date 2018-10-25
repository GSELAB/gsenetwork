#
# Config LLVM JIT

#LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
#MESSAGE(STATUS "JIT just support x86_64 or AMD64")
#INCLUDE(ProjectLLVM)
#configure_llvm_project()


SET(runtime_source
    JITContext.cpp
    Runtime.cpp)

ADD_SUBDIRECTORY(storage)
ADD_SUBDIRECTORY(action)
#ADD_SUBDIRECTORY(vm)
#ADD_SUBDIRECTORY(jit)

ADD_LIBRARY(runtime ${runtime_source})
#TARGET_LINK_LIBRARIES(runtime
#    utils runtime_storage runtime_action database chain core crypto runtime_jit ${GSE_BOOST_LIBS})
TARGET_LINK_LIBRARIES(runtime
    utils runtime_storage runtime_action database chain core crypto ${GSE_BOOST_LIBS})