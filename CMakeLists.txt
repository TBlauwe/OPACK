# CMake configuration 
# =================== 
cmake_minimum_required (VERSION 3.13...3.23)


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
set(CPM_DOWNLOAD_VERSION 0.35.0)
include(cmake/FindCPM.cmake)
include(cmake/target_default_compile_options.cmake)

# TODO : Since we output build files in a bin/ instead of out/, it doesn't semmes to work.
# CCache must be installed and available in your path !
# enables CCACHE support through the USE_CCACHE flag possible values are: YES, NO or equivalent.
#CPMAddPackage(
#  NAME Ccache.cmake
#  GITHUB_REPOSITORY TheLartians/Ccache.cmake
#  VERSION 1.2
#)

# Build options
# =============
option(OPACK_BUILD_EXAMPLES       "Build examples." OFF)
option(OPACK_BUILD_EXPERIMENTS    "Build experiments." OFF)
option(OPACK_BUILD_TESTS          "Build tests." OFF)
option(OPACK_BUILD_BENCHMARKS     "Build benchmarks using google benchmarks." OFF)
option(OPACK_DEVELOPPER_WARNINGS  "Enable more warnings when compiling" OFF)
option(OPACK_ORGANIZE             "Enable organisation of entities, different from C++ namespace (mainly organisation for explorer). Disabling it may lead to more performance." ON)
option(OPACK_ENABLE_RUNTIME_CHECK "Enable runtime check (assertion). Disabling it may lead to more performance." ON)
option(OPACK_ENABLE_LOG           "Enable runtime log. Disabling it may lead to more performance." OFF)


# Dependencies
# ============

# Boost provides free peer-reviewed portable C++ source libraries
#CPMAddPackage(
#  NAME Boost 
#  GITHUB_REPOSITORY "boostorg/boost"
#  GIT_TAG "boost-1.80.0"
#  OPTIONS
#    "BOOST_RUNTIME_LINK static"
#)

# General-purpose Parallel and Heterogeneous Task Programming System
#CPMAddPackage(
#        NAME taskflow
#        GITHUB_REPOSITORY taskflow/taskflow
#        GIT_TAG v3.3.0
#        OPTIONS
#        "TF_BUILD_TESTS OFF"
#        "TF_BUILD_EXAMPLES OFF"
#)
#set_target_properties(Taskflow PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:Taskflow,INTERFACE_INCLUDE_DIRECTORIES>)

# A fast entity component system (ECS) for C & C++
CPMAddPackage(
        NAME flecs
        GITHUB_REPOSITORY SanderMertens/flecs
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
)
set_target_properties(fmt PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:fmt,INTERFACE_INCLUDE_DIRECTORIES>)

# A convenient random API
CPMAddPackage(
		NAME random
		GITHUB_REPOSITORY effolkronium/random
		GIT_TAG master
		OPTIONS
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
add_library (opack STATIC ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(opack PUBLIC include ${random_SOURCE_DIR}/include)
target_link_libraries(opack PUBLIC flecs_static fmt)
if(${OPACK_ENABLE_RUNTIME_CHECK})
    target_compile_definitions(opack PUBLIC OPACK_RUNTIME_CHECK)
endif()
if(${OPACK_ENABLE_LOG})
    target_compile_definitions(opack PUBLIC OPACK_LOG)
endif()
target_compile_features(opack PUBLIC cxx_std_20)
if(OPACK_DEVELOPPER_WARNINGS)
    target_set_compile_warnings_cxx(opack)
endif()

if(OPACK_BUILD_EXAMPLES)
    add_subdirectory ("examples")
endif()

if(OPACK_BUILD_EXPERIMENTS)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		message(WARNING "Matplot++ does not works with clang. Use MSVC. -- skipping experiments")
    else()
		find_package(Gnuplot)
		if(GNUPLOT_FOUND)
			add_subdirectory ("experiments")
		else()
			message(WARNING "GnuPlot not found -- skipping experiments (link : http://www.gnuplot.info/)")
		endif()
    endif()
endif()

if(OPACK_BUILD_BENCHMARKS)
    add_subdirectory ("benchmarks")
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
        message(WARNING "Doxygen not found - skipping docs (link : https://www.doxygen.nl/download.html)")
    endif()
endif()
