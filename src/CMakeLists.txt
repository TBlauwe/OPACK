# ==================================================================
# OPACK - top cmake file
# Tries to follow : https://cliutils.gitlab.io/modern-cmake/
# Any helps, corrections, tips or tricks will be highly appreciated
# ==================================================================
cmake_minimum_required (VERSION 3.12...3.23)

# Header files
set(HEADER_LIST 
	"${OPACK_SOURCE_DIR}/include/opack/core.hpp"
	"${OPACK_SOURCE_DIR}/src/opack.h"
)

# Source files
set(SOURCE_LIST 
	"${OPACK_SOURCE_DIR}/src/OPACK.cpp"
)

add_executable (OPACK ${SOURCE_LIST} ${HEADER_LIST})
