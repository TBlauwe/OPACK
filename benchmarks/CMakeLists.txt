# Library files
# ==============
set(SOURCE_LIST 
	"core/world.cpp"
    "core/perception.cpp"
    "core/action.cpp"
    "module/agents.cpp"
    "utils/ring_buffer.cpp"
)

# Dependencies
# ============
# A microbenchmark support library 
CPMAddPackage(
        NAME benchmark 
        GITHUB_REPOSITORY google/benchmark
        GIT_TAG v1.8.1
        OPTIONS
            "BENCHMARK_ENABLE_TESTING OFF"
)
if(benchmark_ADDED)
    set_target_properties(benchmark PROPERTIES CXX_STANDARD 11) # enable c++11 to avoid compilation errors
    target_compile_options(benchmark PUBLIC -D_CRT_USE_BUILTIN_OFFSETOF)
endif ()

add_executable(opack_benchmarks "utils.hpp" ${SOURCE_LIST})
target_compile_features(opack_benchmarks PRIVATE cxx_std_20)
target_compile_definitions(opack_benchmarks PRIVATE OPACK_OPTIMIZATION)
target_link_libraries(opack_benchmarks PRIVATE opack benchmark::benchmark)
set_target_properties(opack_benchmarks PROPERTIES OUTPUT_NAME "Benchmarks")
set_target_properties(opack_benchmarks PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}")

# Copy compare.py tools and its requirements for ease of use
add_custom_command(TARGET opack_benchmarks POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OPACK_BINARY_OUTPUT_DIR}/benchmarks_tools/"
)
add_custom_command(TARGET opack_benchmarks POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${benchmark_SOURCE_DIR}/tools" "${OPACK_BINARY_OUTPUT_DIR}/benchmarks_tools/"
)
        
# Copy run_benchmarks.py to launch benchmark
add_custom_command(TARGET opack_benchmarks POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${OPACK_SOURCE_DIR}/benchmarks/tools/run_benchmarks.py"
        "${OPACK_BINARY_OUTPUT_DIR}/"
)
