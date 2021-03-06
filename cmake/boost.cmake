

SET(BOOST_CXXFLAGS "")
IF(WIN32)
    SET(BOOST_BOOTSTRAP_COMMAND bootstrap.bat)
    SET(BOOST_BUILD_TOOL b2.exe)
    SET(BOOST_LIBRARY_SUFFIX -vc140-mt-1_63.lib)
ELSE()
    SET(BOOST_BOOTSTRAP_COMMAND ./bootstrap.sh)
    SET(BOOST_BUILD_TOOL ./b2)
    SET(BOOST_LIBRARY_SUFFIX .a)
    IF(${BUILD_SHARED_LIBS})
        SET(BOOST_CXXFLAGS "cxxflags=-fPIC")
    ENDIF()
ENDIF()

