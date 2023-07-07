# Made after "Modern CMake Tutorial" and example at : 
# https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/extended-project/CMakeLists.txt

# CMake configuration 
# =================== 
cmake_minimum_required (VERSION 3.14...3.26)


# Project information
# ===================
project(OPACK
        VERSION 0.5.0
        DESCRIPTION "C++ library to simulate cognitive agents."
        HOMEPAGE_URL "https://github.com/tblauwe/opack"
        LANGUAGES C CXX
)

# CMake libraries
# ===============
# Dependencies are added via CPM.
# See https://github.com/cpm-cmake/CPM.cmake for more info.
set(CPM_DOWNLOAD_VERSION 0.38.1)
include(cmake/cpm.cmake)
include(cmake/target_set_compile_warnings.cmake)
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.11.0")


# Include guards
# ==============

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  include(CTest)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, skipping docs")
  endif()

endif()


# Convience variable so that annexes bin (tests, benchmarks, experiments, examples) are outputed to a nice structure.
set(OPACK_BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/") 
# Build options
# =============
option(OPACK_BUILD_EXAMPLES       "Build examples." OFF)
option(OPACK_BUILD_EXPERIMENTS    "Build experiments." OFF)
option(OPACK_BUILD_BENCHMARKS     "Build benchmarks using google benchmarks." OFF)
option(OPACK_DEVELOPPER_WARNINGS  "Enable more warnings when compiling" OFF)
option(OPACK_ORGANIZE             "Enable organisation of entities, different from C++ namespace (mainly organisation for explorer). Disabling it may lead to more performance." ON)
option(OPACK_ENABLE_RUNTIME_CHECK "Enable runtime check (assertion). Disabling it may lead to more performance." ON)
option(OPACK_ENABLE_LOG           "Enable runtime log. Disabling it may lead to more performance." OFF)


# Dependencies
# ============

# A fast entity component system (ECS) for C & C++
CPMAddPackage(
        NAME flecs
        GITHUB_REPOSITORY SanderMertens/flecs
        #GIT_TAG master
        GIT_TAG v3.1.0
        OPTIONS
        "FLECS_STATIC ON"
        "FLECS_SHARED OFF"
)
set_target_properties(flecs_static PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:flecs_static,INTERFACE_INCLUDE_DIRECTORIES>)

# A modern formatting library
CPMAddPackage(
		NAME fmt 
		GITHUB_REPOSITORY fmtlib/fmt
		GIT_TAG 9.1.0
		OPTIONS
            "FMT_INSTALL ON"
)
set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:fmt,INTERFACE_INCLUDE_DIRECTORIES>)

# A convenient random API
CPMAddPackage(
		NAME random
		GITHUB_REPOSITORY effolkronium/random
		GIT_TAG master
		OPTIONS
            "Random_BuildTests OFF"
)

# OPACK Library
# =============
set(HEADER_LIST 
	"include/opack/utils/type_map.hpp"
    "include/opack/utils/flecs_helper.hpp" 
    "include/opack/utils/debug.hpp" 
    "include/opack/utils/type_name.hpp"
    "include/opack/core/macros.hpp"
    "include/opack/core/api_types.hpp"
    "include/opack/core/components.hpp"
    "include/opack/core/world.hpp"
    "include/opack/core/entity.hpp" 
	"include/opack/core/operation.hpp"
    "include/opack/core/perception.hpp"
    "include/opack/core/action.hpp" 
    "include/opack/core/communication.hpp" 
	"include/opack/core.hpp"
    "include/opack/operations/basic.hpp" 
    "include/opack/operations/influence_graph.hpp" 
    "include/opack/module/fipa_acl.hpp" 
    "include/opack/module/adl.hpp"
    "include/opack/module/flows.hpp"
    "include/opack/module/simple_agent.hpp"
 )

set(SOURCE_LIST 
    "src/core.cpp" 
    "src/core/communication.cpp"
    "src/core/perception.cpp" 
	"src/core/simulation.cpp"
    "src/module/fipa_acl.cpp" 
    "src/module/activity_dl.cpp"
    )


# Target : opack
# ==============
add_library (opack ${SOURCE_LIST} ${HEADER_LIST})
target_compile_features(opack PUBLIC cxx_std_20)
# using __VA_OPT__, we enforce latest preprocess on MSVC
target_compile_options(opack PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>)
# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(opack PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_include_directories(opack PUBLIC include ${random_SOURCE_DIR}/include)
target_link_libraries(opack PUBLIC flecs_static fmt)

if(${OPACK_ENABLE_RUNTIME_CHECK})
    target_compile_definitions(opack PUBLIC OPACK_RUNTIME_CHECK)
endif()
if(${OPACK_ENABLE_LOG})
    target_compile_definitions(opack PUBLIC OPACK_LOG)
endif()
if(${OPACK_DEVELOPPER_WARNINGS})
    target_set_compile_warnings_cxx(opack)
endif()


# Other targets
# =============

if(OPACK_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(OPACK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(OPACK_BUILD_EXPERIMENTS)
    add_subdirectory(experiments)
endif()

# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
