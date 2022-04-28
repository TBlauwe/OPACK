# Library files
# ==============
set(SOURCE_LIST 
	"console.cpp"
)

set(HEADER_LIST 
	"include/opack/examples/empty_sim.hpp"
	"include/opack/examples/simple_sim.hpp"
)

add_library(opack_examples INTERFACE ${HEADER_LIST})
target_link_libraries(opack_examples INTERFACE opack)
target_include_directories(opack_examples INTERFACE "include")

add_executable(console ${SOURCE_LIST})
target_compile_features(console PRIVATE cxx_std_20)
target_link_libraries(console PRIVATE opack opack_examples)
set_target_properties(console PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples/$<CONFIG>")

add_executable(test_bench "test_bench.cpp")
target_link_libraries(test_bench PRIVATE flecs_static)
set_target_properties(test_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples/$<CONFIG>")