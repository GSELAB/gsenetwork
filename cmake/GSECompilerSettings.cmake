INCLUDE(GSECheckCXXCompilerFlag)

gse_add_cxx_compiler_flag_if_supported(-fstack-protector-strong have_stack_protector_strong_support)
IF(NOT have_stack_protector_strong_support)
    gse_add_cxx_compiler_flag_if_supported(-fstack-protector)
ENDIF()

gse_add_cxx_compiler_flag_if_supported(-Wimplicit-fallthrough)

IF(("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
    # Enables all the warnings about constructions that some users consider questionable,
    # and that are easy to avoid.  Also enable some extra warning flags that are not
    # enabled by -Wall.   Finally, treat at warnings-as-errors, which forces developers
    # to fix warnings as they arise, so they don't accumulate "to be fixed later".
    #ADD_COMPILE_OPTIONS(-Wall)
    #ADD_COMPILE_OPTIONS(-Wextra)
    #ADD_COMPILE_OPTIONS(-Werror)  # this may cause warning to error <Jorge>

    # Disable warnings about unknown pragmas (which is enabled by -Wall).
    ADD_COMPILE_OPTIONS(-Wno-unknown-pragmas)

    # Configuration-specific compiler settings.
    SET(CMAKE_CXX_FLAGS_DEBUG          "-Og -g -DGSE_DEBUG")
    SET(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    SET(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

    OPTION(USE_LD_GOLD "Use GNU gold linker" ON)
    IF(USE_LD_GOLD)
        EXECUTE_PROCESS(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
        IF("${LD_VERSION}" MATCHES "GNU gold")
            SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
            SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
        ENDIF()
    ENDIF()

    # Hide all symbols by default.
    ADD_COMPILE_OPTIONS(-fvisibility=hidden)
    IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # Do not export symbols from dependencies.
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL")
    ENDIF()

    # Check GCC compiler version.
    IF("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
        IF("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 5)
            MESSAGE(FATAL_ERROR "This compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} is not supported. GCC 5 or newer is required.")
        ENDIF()

    # Stop if buggy clang compiler detected.
    ELSEIF("${CMAKE_CXX_COMPILER_ID}" MATCHES AppleClang)
        IF("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 8.4)
            message(FATAL_ERROR "This compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} is not able to compile required libff. Install clang 4+ from Homebrew or XCode 9.")
        ENDIF()
    ENDIF()

# The major alternative compiler to GCC/Clang is Microsoft's Visual C++ compiler, only available on Windows.
ELSEIF(MSVC)

    ADD_COMPILE_OPTIONS(/MP)                        # enable parallel compilation
    ADD_COMPILE_OPTIONS(/EHsc)                        # specify Exception Handling Model in msvc
    ADD_COMPILE_OPTIONS(/WX)                        # enable warnings-as-errors
    ADD_COMPILE_OPTIONS(/wd4068)                    # disable unknown pragma warning (4068)
    ADD_COMPILE_OPTIONS(/wd4996)                    # disable unsafe function warning (4996)
    ADD_COMPILE_OPTIONS(/wd4503)                    # disable decorated name length exceeded, name was truncated (4503)
    ADD_COMPILE_OPTIONS(/wd4267)                    # disable conversion from 'size_t' to 'type', possible loss of data (4267)
    ADD_COMPILE_OPTIONS(/wd4180)                    # disable qualifier applied to function type has no meaning; ignored (4180)
    ADD_COMPILE_OPTIONS(/wd4290)                    # disable C++ exception specification ignored except to indicate a function is not __declspec(nothrow) (4290)
    ADD_COMPILE_OPTIONS(/wd4297)                    # disable <vector>'s function assumed not to throw an exception but does (4297)
    ADD_COMPILE_OPTIONS(/wd4244)                    # disable conversion from 'type1' to 'type2', possible loss of data (4244)
    ADD_COMPILE_OPTIONS(/wd4800)                    # disable forcing value to bool 'true' or 'false' (performance warning) (4800)
    ADD_COMPILE_OPTIONS(-D_WIN32_WINNT=0x0600)        # declare Windows Vista API requirement
    ADD_COMPILE_OPTIONS(-DNOMINMAX)                    # undefine windows.h MAX && MIN macros cause it cause conflicts with std::min && std::max functions
    ADD_COMPILE_OPTIONS(-DMINIUPNP_STATICLIB)        # define miniupnp static library

    # Always use Release variant of C++ runtime.
    # We don't want to provide Debug variants of all dependencies. Some default
    # flags set by CMake must be tweaked.
    STRING(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    STRING(REPLACE "/D_DEBUG" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    STRING(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    STRING(REPLACE "/MDd" "/MD" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    STRING(REPLACE "/D_DEBUG" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    STRING(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    SET_PROPERTY(GLOBAL PROPERTY DEBUG_CONFIGURATIONS OFF)

    # disable empty object file warning
    SET(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4221")
    # warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/SAFESEH' specification
    # warning LNK4099: pdb was not found with lib
    # stack size 16MB
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099,4075 /STACK:16777216")

# If you don't have GCC, Clang or VC++ then you are on your own.  Good luck!
ELSE()
    MESSAGE(WARNING "Your compiler is not tested, if you run into any issues, we'd welcome any patches.")
ENDIF()

IF(SANITIZE)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZE}")
    IF(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize-blacklist=${CMAKE_SOURCE_DIR}/sanitizer-blacklist.txt")
    ENDIF()
ENDIF()

OPTION(COVERAGE "Build with code coverage support" OFF)
IF(COVERAGE)
    ADD_COMPILE_OPTIONS(-g --coverage)
    SET(CMAKE_SHARED_LINKER_FLAGS "--coverage ${CMAKE_SHARED_LINKER_FLAGS}")
    SET(CMAKE_EXE_LINKER_FLAGS "--coverage ${CMAKE_EXE_LINKER_FLAGS}")
ENDIF()

IF(UNIX AND NOT APPLE)
    OPTION(STATIC_LIBSTDCPP "Link libstdc++ staticly")
    IF(STATIC_LIBSTDCPP)
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
    ENDIF()
ENDIF()
