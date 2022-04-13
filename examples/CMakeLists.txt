# Library files
# ==============
set(SOURCE_LIST 
	"console.cpp"
)

set(HEADER_LIST 
	"simple_sim.hpp"
	"empty_sim.hpp"
)

add_executable(console ${SOURCE_LIST} ${HEADER_LIST})
target_compile_features(console PRIVATE cxx_std_20)
target_link_libraries(console PRIVATE opack)
set_target_properties(console PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_SOURCE_DIR}/bin/examples")