# 3rd-party libraries
# ===================
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
add_subdirectory(01_sensibility)
