# Library files
# ==============
set(HEADER_LIST 
	"${OPACK_SOURCE_DIR}/include/opack/core.hpp"
)

set(SOURCE_LIST 
	"${OPACK_SOURCE_DIR}/src/core/simulation.cpp"
)

# Dependencies
# ============

# General-purpose Parallel and Heterogeneous Task Programming System
CPMAddPackage(
        NAME taskflow
        GITHUB_REPOSITORY taskflow/taskflow
        GIT_TAG v3.3.0
        OPTIONS
        "TF_BUILD_TESTS OFF"
        "TF_BUILD_EXAMPLES OFF"
)
set_target_properties(Taskflow PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:Taskflow,INTERFACE_INCLUDE_DIRECTORIES>)

# A fast entity component system (ECS) for C & C++
CPMAddPackage(
        NAME flecs
        GITHUB_REPOSITORY SanderMertens/flecs
        GIT_TAG master
        OPTIONS
        "FLECS_STATIC_LIBS ON"
        "FLECS_SHARED_LIBS OFF"
)
set_target_properties(flecs_static PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:flecs_static,INTERFACE_INCLUDE_DIRECTORIES>)

# Target : opack
# ==============
add_library (opack ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(opack PUBLIC "${OPACK_SOURCE_DIR}/include")
target_link_libraries(opack PUBLIC Taskflow flecs_static)
target_compile_features(opack PUBLIC cxx_std_20)

if(OPACK_DEVELOPPER_WARNINGS)
	target_default_compile_warnings_cxx(opack)
endif()
