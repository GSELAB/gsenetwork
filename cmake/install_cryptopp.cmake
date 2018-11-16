include("${CMAKE_CURRENT_LIST_DIR}/src/helpers.cmake")

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(INSTALL_DIR "${ROOT_DIR}/install")

# https://github.com/GSELAB/cpp-deps/releases/download/cryptopp/cryptopp-CRYPTOPP_6_0_0.tar.gz
set(SERVER "https://github.com/GSELAB/cpp-deps/releases/download/cryptopp/")


function(download_and_install PACKAGE_NAME)
    download_and_unpack("${SERVER}${PACKAGE_NAME}.tar.gz" ${INSTALL_DIR})
endfunction(download_and_install)

download_and_install("cryptopp-CRYPTOPP_6_0_0")