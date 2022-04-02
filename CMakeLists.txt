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
option(OPACK_DEVELOPPER_WARNINGS   "Enable more warnings" OFF)


# OPACK Library
# =============
add_subdirectory ("src")

if(OPACK_BUILD_TESTS)
    add_subdirectory ("tests")
endif()

if(OPACK_BUILD_BENCHMARKS)
    add_subdirectory ("benchmarks")
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

