# Targets
# =======
add_executable(experiment_02_shelling "main.cpp" "src/environment.hpp" "src/component.hpp" "src/model.hpp")
target_compile_features(experiment_02_shelling PRIVATE cxx_std_20)
target_link_libraries(experiment_02_shelling PRIVATE opack)
set_target_properties(experiment_02_shelling PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}/experiments/02_shelling")
#configure_file(
#	"${CMAKE_CURRENT_LIST_DIR}/configuration.flecs}" 
#	"${OPACK_BINARY_OUTPUT_DIR}/experiments/02_shelling/plecs/configuration.flecs}"
#)
