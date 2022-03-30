# ==================================================================
# Project cmake file
# Tries to follow : https://cliutils.gitlab.io/modern-cmake/
# Any helps, corrections, tips or tricks will be highly appreciated
# ==================================================================

# CMake configuration 
# =================== 
cmake_minimum_required (VERSION 3.12...3.23)

# Project information
# -------------------
project(OPACK
        VERSION 0.1.0
        DESCRIPTION "C++ library to simulate cognitive agents."
        HOMEPAGE_URL "https://github.com/tblauwe/opack"
        LANGUAGES CXX
)

# Build options
# -------------
option(OPACK_BUILD_APPS           "Build several samples to showcase the library." OFF)
option(OPACK_BUILD_BENCHMARKS     "Build benchmarks using google benchmarks." OFF)
option(OPACK_BUILD_TESTS          "Build tests." OFF)

# Incluez les sous-projets.
add_subdirectory ("src")

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(WARNING "Doxygen not found, not building docs")
        MESSAGE(WARNING "Doxygen link : https://www.doxygen.nl/download.html")
    endif()
endif()

