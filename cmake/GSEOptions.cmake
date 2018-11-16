MACRO(CONFIGURE_PROJECT)
    # Default to RelWithDebInfo configuration if no configuration is explicitly specified.
    IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
       set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type on single-configuration generators" FORCE)
    ENDIF()

    OPTION(BUILD_SHARED_LIBS "Build project libraries shared" OFF)

    # Features:
    OPTION(VMTRACE "Enable VM tracing" OFF)
    OPTION(EVM_OPTIMIZE "Enable VM optimizations (can distort tracing)" ON)
    OPTION(FATDB "Enable fat state database" ON)
    OPTION(PARANOID "Enable additional checks when validating transactions (deprecated)" OFF)
    OPTION(MINIUPNPC "Build with UPnP support" OFF)
    OPTION(FASTCTEST "Enable fast ctest" OFF)

    IF(MINIUPNPC)
        MESSAGE(WARNING
            "Security vulnerabilities have been discovered in miniupnpc library. "
            "This build option is for testing only. Do not use it in public networks")
    ENDIF()

    # components
    OPTION(TESTS "Build with tests" ON)
    OPTION(TOOLS "Build additional tools" ON)
    OPTION(EVMJIT "Build with EVMJIT module enabled" OFF)
    OPTION(HERA "Build with HERA module enabled" OFF)

    # Resolve any clashes between incompatible options.
    IF("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        IF(PARANOID)
            MESSAGE(WARNING "Paranoia requires debug - disabling for release build.")
            SET(PARANOID OFF)
        ENDIF()
        IF(VMTRACE)
            MESSAGE(WARNING "VM Tracing requires debug - disabling for release build.")
            SET(VMTRACE OFF)
        ENDIF()
    ENDIF()

    # FATDB is an option to include the reverse hashes for the trie,
    # i.e. it allows you to iterate over the contents of the state.
    IF(FATDB)
        ADD_DEFINITIONS(-DGSE_FATDB)
    ENDIF()

    IF(PARANOID)
        ADD_DEFINITIONS(-DGSE_PARANOIA)
    ENDIF()

    IF(VMTRACE)
        ADD_DEFINITIONS(-DGSE_VMTRACE)
    ENDIF()

    # CI Builds should provide (for user builds this is totally optional)
    # -DBUILD_NUMBER - A number to identify the current build with. Becomes TWEAK component of project version.
    # -DVERSION_SUFFIX - A string to append to the end of the version string where applicable.
    IF(NOT DEFINED BUILD_NUMBER)
        # default is big so that local build is always considered greater
        # and can easily replace CI build for for all platforms if needed.
        # Windows max version component number is 65535
        SET(BUILD_NUMBER 65535)
    ENDIF()

    # Suffix like "-rc1" e.t.c. to append to versions wherever needed.
    IF(NOT DEFINED VERSION_SUFFIX)
        SET(VERSION_SUFFIX "")
    ENDIF()

    PRINT_CONFIGURATION()
ENDMACRO()

MACRO(PRINT_CONFIGURATION)
    MESSAGE(STATUS "Configuring ${PROJECT_NAME}")
    MESSAGE(STATUS "CMake ${CMAKE_VERSION} (${CMAKE_COMMAND})")
    MESSAGE(STATUS "CMAKE_BUILD_TYPE Build type ${CMAKE_BUILD_TYPE}")
    MESSAGE(STATUS "TARGET_PLATFORM  Target platform                          ${CMAKE_SYSTEM_NAME}")
    MESSAGE(STATUS "BUILD_SHARED_LIBS                                         ${BUILD_SHARED_LIBS}")
    MESSAGE(STATUS "<features>")
    MESSAGE(STATUS "FATDB            Full database exploring                  ${FATDB}")
    MESSAGE(STATUS "DB               Database implementation                  LEVELDB")
    MESSAGE(STATUS "PARANOID         -                                        ${PARANOID}")
    MESSAGE(STATUS "MINIUPNPC        -                                        ${MINIUPNPC}")
    MESSAGE(STATUS "<components>")
    MESSAGE(STATUS "TESTS Build tests ${TESTS}")
    MESSAGE(STATUS "TOOLS            Build tools                              ${TOOLS}")
    MESSAGE(STATUS "HERA             Build Hera eWASM VM                      ${HERA}")
ENDMACRO()
