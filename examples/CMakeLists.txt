# Library files
# ==============
set(OPACK_EXAMPLES
	00_empty
	01_simple_agent
)

set(EXAMPLE_OUTPUT_DIR "${OPACK_SOURCE_DIR}/bin/examples")
foreach(example IN LISTS OPACK_EXAMPLES)

	add_executable(${example} ${example}.cpp)
	target_compile_features(${example} PRIVATE cxx_std_20)
	target_link_libraries(${example} PRIVATE opack)
	set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTPUT_DIR})

	add_custom_command(OUTPUT ${example} ${CMAKE_COMMAND} -E make_directory "${EXAMPLE_OUTPUT_DIR}/plecs")
	add_custom_command(TARGET ${example}
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${OPACK_SOURCE_DIR}/examples/plecs" "${EXAMPLE_OUTPUT_DIR}/plecs")
endforeach()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_executable(test_bench "test_bench.cpp")
	target_compile_features(test_bench PRIVATE cxx_std_20)
	target_compile_options(test_bench PRIVATE -fms-compatibility-version=0)
	target_link_libraries(test_bench PRIVATE flecs_static)
	set_target_properties(test_bench PROPERTIES OUTPUT_NAME test_bench_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
	set_target_properties(test_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTPUT_DIR})
endif()

