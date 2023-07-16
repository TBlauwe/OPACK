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


# Targets
# =======
add_executable(experiment_02_shelling "main.cpp" "src/environment.hpp" "src/identifiers.hpp" "src/model.hpp" "src/display.hpp")
target_compile_features(experiment_02_shelling PRIVATE cxx_std_20)
target_link_libraries(experiment_02_shelling PRIVATE opack)
set_target_properties(experiment_02_shelling PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}/experiments/02_shelling")
#configure_file(
#	"${CMAKE_CURRENT_LIST_DIR}/configuration.flecs}" 
#	"${OPACK_BINARY_OUTPUT_DIR}/experiments/02_shelling/plecs/configuration.flecs}"
#)

add_executable(experiment_02_shelling_benchmarks "benchmark.cpp" "src/environment.hpp" "src/identifiers.hpp" "src/model.hpp" "src/display.hpp")
target_compile_features(experiment_02_shelling_benchmarks PRIVATE cxx_std_20)
target_compile_definitions(experiment_02_shelling_benchmarks PRIVATE OPACK_OPTIMIZATION)
target_link_libraries(experiment_02_shelling_benchmarks PRIVATE opack benchmark::benchmark)
set_target_properties(experiment_02_shelling_benchmarks PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}/experiments/02_shelling")

