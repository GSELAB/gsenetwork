#
# renames the file if it is different from its destination
INCLUDE(CMakeParseArguments)
#
MACRO(replace_if_different SOURCE DST)
	SET(extra_macro_args ${ARGN})
	SET(options CREATE)
	SET(one_value_args)
	SET(multi_value_args)
	cmake_parse_arguments(REPLACE_IF_DIFFERENT "${options}" "${one_value_args}" "${multi_value_args}" "${extra_macro_args}")

	IF(REPLACE_IF_DIFFERENT_CREATE AND (NOT (EXISTS "${DST}")))
		file(WRITE "${DST}" "")
	ENDIF()

	execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${SOURCE}" "${DST}" RESULT_VARIABLE DIFFERENT OUTPUT_QUIET ERROR_QUIET)

	IF(DIFFERENT)
		execute_process(COMMAND ${CMAKE_COMMAND} -E rename "${SOURCE}" "${DST}")
	ELSE()
		execute_process(COMMAND ${CMAKE_COMMAND} -E remove "${SOURCE}")
	ENDIF()
ENDMACRO()

MACRO(gse_add_test NAME)

	# parse arguments here
	SET(commands)
	SET(current_command "")
	FOREACH(arg ${ARGN})
		IF(arg STREQUAL "ARGS")
			IF(current_command)
				LIST(APPEND commands ${current_command})
			ENDIF()
			SET(current_command "")
		ELSE()
			SET(current_command "${current_command} ${arg}")
		ENDIF()
	ENDFOREACH(arg)
	LIST(APPEND commands ${current_command})

	MESSAGE(STATUS "test: ${NAME} | ${commands}")

	# create tests
	SET(index 0)
	LIST(LENGTH commands count)
	WHILE(index LESS count)
		LIST(GET commands ${index} test_arguments)

		SET(run_test "--run_test=${NAME}")
		add_test(NAME "${NAME}.${index}" COMMAND testgse ${run_test} ${test_arguments})
		
		math(EXPR index "${index} + 1")
	ENDWHILE(index LESS count)

	# add target to run them
	ADD_CUSTOM_TARGET("test.${NAME}"
		DEPENDS testgse
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} -DETH_TEST_NAME="${NAME}" -DCTEST_COMMAND="${CTEST_COMMAND}" -P "${ETH_SCRIPTS_DIR}/runtest.cmake"
	)

ENDMACRO()

# In Windows split repositories build we need to be checking whether or not
# Debug/Release or both versions were built for the config phase to run smoothly
MACRO(gse_check_library_link L)
	IF(${${L}_LIBRARY} AND ${${L}_LIBRARY} EQUAL "${L}_LIBRARY-NOTFOUND")
		unset(${${L}_LIBRARY})
	ENDIF()
	IF(${${L}_LIBRARY_DEBUG} AND ${${L}_LIBRARY_DEBUG} EQUAL "${L}_LIBRARY_DEBUG-NOTFOUND")
		unset(${${L}_LIBRARY_DEBUG})
	ENDIF()
	IF(${${L}_LIBRARY} AND ${${L}_LIBRARY_DEBUG})
		SET(${L}_LIBRARIES optimized ${${L}_LIBRARY} debug ${${L}_LIBRARY_DEBUG})
	ELSEIF(${${L}_LIBRARY})
		SET(${L}_LIBRARIES ${${L}_LIBRARY})
	ELSEIF(${${L}_LIBRARY_DEBUG})
		SET(${L}_LIBRARIES ${${L}_LIBRARY_DEBUG})
	ENDIF()
ENDMACRO()

