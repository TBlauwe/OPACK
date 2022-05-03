# Library files
# ==============
set(SOURCE_LIST 
	"basic.cpp"
)

# Dependencies
# ============
# A microbenchmark support library 
CPMAddPackage(
        NAME benchmark 
        GITHUB_REPOSITORY google/benchmark
        GIT_TAG v1.6.1
        OPTIONS
            "BENCHMARK_ENABLE_TESTING OFF"
)
if(benchmark_ADDED)
    set_target_properties(benchmark PROPERTIES CXX_STANDARD 11) # enable c++11 to avoid compilation errors
endif ()

add_executable(opack_benchmarks ${SOURCE_LIST})
target_compile_features(opack_benchmarks PRIVATE cxx_std_20)
target_link_libraries(opack_benchmarks PRIVATE opack opack_examples benchmark::benchmark)
set_target_properties(opack_benchmarks PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/$<CONFIG>/benchmark")

# Copy compare.py tools and its requirements for ease of use
add_custom_command(TARGET opack_benchmarks POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:opack_benchmarks>/tools)
add_custom_command(TARGET opack_benchmarks POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${benchmark_SOURCE_DIR}/tools $<TARGET_FILE_DIR:opack_benchmarks>/tools)
