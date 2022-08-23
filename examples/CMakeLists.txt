# Library files
# ==============
set(HEADER_LIST 
	"include/opack/examples/empty_sim.hpp"
	"include/opack/examples/simple_sim.hpp"
 "include/opack/examples/all.hpp" "include/opack/examples/medical_sim.hpp")

add_library(opack_examples INTERFACE ${HEADER_LIST})
target_link_libraries(opack_examples INTERFACE opack)
target_include_directories(opack_examples INTERFACE "include")

add_executable(console "console.cpp")
target_compile_features(console PRIVATE cxx_std_20)
target_link_libraries(console PRIVATE opack opack_examples)
set_target_properties(console PROPERTIES OUTPUT_NAME console_${CMAKE_PROJECT_VERSION}_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
set_target_properties(console PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")

add_executable(manual_console "manual_console.cpp")
target_compile_features(manual_console PRIVATE cxx_std_20)
target_link_libraries(manual_console PRIVATE opack opack_examples)
set_target_properties(manual_console PROPERTIES OUTPUT_NAME manual_console_${CMAKE_PROJECT_VERSION}_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
set_target_properties(manual_console PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_executable(test_bench "test_bench.cpp")
	target_compile_features(test_bench PRIVATE cxx_std_20)
	target_compile_options(test_bench PRIVATE -fms-compatibility-version=0)
	target_link_libraries(test_bench PRIVATE flecs_static)
	set_target_properties(test_bench PROPERTIES OUTPUT_NAME test_bench_${CMAKE_PROJECT_VERSION}_$<CONFIG>_${CMAKE_CXX_COMPILER_ID})
	set_target_properties(test_bench PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")
endif()