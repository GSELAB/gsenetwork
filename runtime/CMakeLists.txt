#
# Config LLVM JIT

#LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
#MESSAGE(STATUS "JIT just support x86_64 or AMD64")
#INCLUDE(ProjectLLVM)
#configure_llvm_project()


ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(storage)
ADD_SUBDIRECTORY(action)
ADD_SUBDIRECTORY(correlator)
#ADD_SUBDIRECTORY(vm)
#ADD_SUBDIRECTORY(jit)

#ADD_LIBRARY(runtime)
#TARGET_LINK_LIBRARIES(runtime
#    utils runtime_storage runtime_action database chain core crypto runtime_jit ${GSE_BOOST_LIBS})
#TARGET_LINK_LIBRARIES(runtime
#    utils runtime_common runtime_storage runtime_action database chain core crypto ${GSE_BOOST_LIBS})