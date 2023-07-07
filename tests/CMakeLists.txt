# Library files
# ==============
set(SOURCE_LIST 
	"main.cpp"
    "utils/ring_buffer.cpp"
    "core/types.cpp"
    "core/basic.cpp"
    "core/simulation.cpp"
    "core/perception.cpp"
    "core/action.cpp"
    "core/operation.cpp" 
    "algorithm/influence_graph.cpp" 
    
    "module/activity_dl.cpp"
    "core/communication.cpp"
)

# Dependencies
# ============
# C++ testing framework
CPMAddPackage(
        NAME doctest
        GITHUB_REPOSITORY onqtam/doctest
        GIT_TAG v2.4.9
)

add_executable(opack_tests ${SOURCE_LIST} "utils.hpp")
target_compile_features(opack_tests PRIVATE cxx_std_20)
target_link_libraries(opack_tests PRIVATE opack doctest::doctest)
set_target_properties(opack_tests PROPERTIES OUTPUT_NAME "Tests")
set_target_properties(opack_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}")
add_test(NAME opack::doctest_all COMMAND tests)

# include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake) # Doesn't seem to work :/