# Setup ccache.
#
# The ccache is auto-enabled if the tool is found.
# To disable set -DCCACHE=OFF option.
IF(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
    FIND_PROGRAM(CCACHE ccache DOC "ccache tool path; set to OFF to disable")
    IF(CCACHE)
        SET(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
        IF(COMMAND cotire)
            # Change ccache config to meet cotire requirements.
            SET(ENV{CCACHE_SLOPPINESS} pch_defines,time_macros)
        ENDIF()
        MESSAGE(STATUS "[ccache] Enabled: ${CCACHE}")
    ENDIF()
ENDIF()
