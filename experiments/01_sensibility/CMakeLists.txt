# 3rd-party libraries
# ===================
# Experiment 1 : Colors
# Add color classes
CPMAddPackage(
        NAME color
        GITHUB_REPOSITORY dmilos/color
        GIT_TAG origin/master
)

# Targets
# =======
add_executable(experiment_01_sensibility "01_sensibility.cpp")
target_compile_features(experiment_01_sensibility PRIVATE cxx_std_20)
target_include_directories(experiment_01_sensibility PRIVATE "${color_SOURCE_DIR}/src/")
target_link_libraries(experiment_01_sensibility PRIVATE matplot opack)
set_target_properties(experiment_01_sensibility PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/experiments/01_sensibility")
configure_file(
    "${OPACK_SOURCE_DIR}/experiments/01_sensibility/configuration.flecs" 
    "${OPACK_SOURCE_DIR}/bin/experiments/01_sensibility/configuration.flecs"
)
