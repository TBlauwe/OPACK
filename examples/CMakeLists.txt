# Library files
# ==============
set(OPACK_EXAMPLES
	00_empty
	01_simple_agent
)

foreach(example IN LISTS OPACK_EXAMPLES)
	add_executable(${example} ${example}.cpp)
	target_compile_features(${example} PRIVATE cxx_std_20)
	target_link_libraries(${example} PRIVATE opack)
	set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")
endforeach()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_executable(test_bench "test_bench.cpp")
	target_compile_features(test_bench PRIVATE cxx_std_20)
	target_compile_options(test_bench PRIVATE -fms-compatibility-version=0)
	target_link_libraries(test_bench PRIVATE flecs_static)
	set_target_properties(test_bench PROPERTIES OUTPUT_NAME test_bench_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
	set_target_properties(test_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")
endif()