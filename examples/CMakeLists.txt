# Library files
# ==============
set(OPACK_EXAMPLES
	00_empty
	01_simulation_controls
	02_world
	03_simple_agent
	04_activity
	05_perception
	06_action
)

set(EXAMPLES_PLECS
	activity.flecs
	perception.flecs
	simple_agent.flecs
)

set(EXAMPLE_OUTPUT_DIR "${OPACK_SOURCE_DIR}/bin/examples")
foreach(example IN LISTS OPACK_EXAMPLES)

	add_executable(${example} ${example}.cpp)
	target_compile_features(${example} PRIVATE cxx_std_20)
	target_link_libraries(${example} PRIVATE opack)
	set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTPUT_DIR})
endforeach()

foreach(plecs IN LISTS EXAMPLES_PLECS)
	configure_file(
			"${OPACK_SOURCE_DIR}/examples/plecs/${plecs}" 
			"${EXAMPLE_OUTPUT_DIR}/plecs/${plecs}")
endforeach()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_executable(test_bench "test_bench.cpp")
	target_compile_features(test_bench PRIVATE cxx_std_11)
	target_link_libraries(test_bench PRIVATE flecs_static)
	set_target_properties(test_bench PROPERTIES OUTPUT_NAME test_bench_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
	set_target_properties(test_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_OUTPUT_DIR})
endif()

