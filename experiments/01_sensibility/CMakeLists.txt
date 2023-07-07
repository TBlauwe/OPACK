if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	message(WARNING "Matplot++ does not work with clang. Use MSVC. -- skipping experiments.")
	return()
endif()

find_package(Gnuplot)
if(GNUPLOT_FOUND)
	message(WARNING "GnuPlot not found -- skipping experiments (link : http://www.gnuplot.info/).")
    return()
endif()

# 3rd-party libraries
# ===================
# Add color classes
CPMAddPackage(
        NAME color
        GITHUB_REPOSITORY dmilos/color
        GIT_TAG origin/master
)

# Add visualization (needs GnuPlot)
CPMAddPackage(
    NAME matplot
    GITHUB_REPOSITORY alandefreitas/matplotplusplus
    GIT_TAG v1.1.0
	OPTIONS
		"BUILD_WITH_PEDANTIC_WARNINGS OFF"
)
set_target_properties(matplot PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:matplot,INTERFACE_INCLUDE_DIRECTORIES>)

# Targets
# =======
add_executable(experiment_01_sensibility "main.cpp")
target_compile_features(experiment_01_sensibility PRIVATE cxx_std_20)
target_include_directories(experiment_01_sensibility PRIVATE "${color_SOURCE_DIR}/src/")
target_link_libraries(experiment_01_sensibility PRIVATE matplot opack)
set_target_properties(experiment_01_sensibility PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OPACK_BINARY_OUTPUT_DIR}/experiments/01_sensibility")
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/configuration.flecs}" 
	"${OPACK_BINARY_OUTPUT_DIR}/experiments/01_sensibility/plecs/configuration.flecs}"
)
