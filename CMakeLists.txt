# CMake configuration 
# =================== 
cmake_minimum_required (VERSION 3.13...3.23)


# Project information
# ===================
project(OPACK
        VERSION 0.1.0
        DESCRIPTION "C++ library to simulate cognitive agents."
        HOMEPAGE_URL "https://github.com/tblauwe/opack"
        LANGUAGES C CXX
)


# CMake libraries
# ===============
set(CPM_DOWNLOAD_VERSION 0.35.0)
include(cmake/FindCPM.cmake)
include(cmake/target_default_compile_options.cmake)


# Build options
# =============
option(OPACK_BUILD_BENCHMARKS     "Build benchmarks using google benchmarks." OFF)
option(OPACK_BUILD_TESTS          "Build tests." OFF)
option(OPACK_BUILD_EXAMPLES       "Build examples." OFF)
option(OPACK_DEVELOPPER_WARNINGS   "Enable more warnings" OFF)


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


# OPACK Library
# =============
set(HEADER_LIST 
	"include/opack/core.hpp"
	"include/opack/utils/type_map.hpp"
)

set(SOURCE_LIST 
	"src/core/simulation.cpp"
)


# Target : opack
# ==============
add_library (opack STATIC ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(opack PUBLIC include)
target_link_libraries(opack PUBLIC Taskflow flecs_static)
target_compile_features(opack PUBLIC cxx_std_20)
if(OPACK_DEVELOPPER_WARNINGS)
    target_set_compile_warnings_cxx(opack)
endif()

if(OPACK_BUILD_BENCHMARKS)
    add_subdirectory ("benchmarks")
endif()

if(OPACK_BUILD_EXAMPLES)
    add_subdirectory ("examples")
endif()

if(OPACK_BUILD_TESTS)
    add_subdirectory ("tests")
endif()


# Documentation
# =============
# * Docs only available if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(WARNING "Doxygen not found, not building docs")
        MESSAGE(WARNING "Doxygen link : https://www.doxygen.nl/download.html")
    endif()
endif()

