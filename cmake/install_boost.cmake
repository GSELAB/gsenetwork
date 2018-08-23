include("${CMAKE_CURRENT_LIST_DIR}/src/helpers.cmake")

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(INSTALL_DIR "${ROOT_DIR}/install")

# https://github.com/GSELAB/cpp-deps/releases/download/master-deps/boost_1_63_0.tar.gz
set(SERVER "https://github.com/GSELAB/cpp-deps/releases/download/master-deps/")


function(download_and_install PACKAGE_NAME)
    download_and_unpack("${SERVER}${PACKAGE_NAME}.tar.gz" ${INSTALL_DIR})
endfunction(download_and_install)

download_and_install("boost_1_63_0")

#execute_process(COMMAND cd install/boost_1_63_0
#                COMMAND ./bootstrap.sh
#                COMMAND cd ../..)

#execute_process(COMMAND ./install/boost_1_63_0/bootstrap.sh WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/boost_1_63_0/)