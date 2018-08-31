INCLUDE(CheckCXXCompilerFlag)

# Adds CXX compiler flag if the flag is supported by the compiler.
#
# This is effectively a combination of CMake's check_cxx_compiler_flag()
# and add_compile_options():
#
#    if(check_cxx_compiler_flag(flag))
#        add_compile_options(flag)
#
FUNCTION(gse_add_cxx_compiler_flag_if_supported FLAG)
    # Remove leading - or / from the flag name.
    STRING(REGEX REPLACE "^-|/" "" name ${FLAG})
    check_cxx_compiler_flag(${FLAG} ${name})
    IF(${name})
        ADD_COMPILE_OPTIONS(${FLAG})
    ENDIF()

    # If the optional argument passed, store the result there.
    IF(ARGV1)
        SET(${ARGV1} ${name} PARENT_SCOPE)
    ENDIF()
ENDFUNCTION()
