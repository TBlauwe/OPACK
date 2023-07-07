# Dependencies
# ============
CPMAddPackage(
        NAME m.css 
        GITHUB_REPOSITORY mosra/m.css
        GIT_TAG master
        OPTIONS
)

# Doxygen setup
# =============
set(DOXYGEN_PROJECT_NAME OPACK)
set(DOXYGEN_PROJECT_VERSION ${OPACK_VERSION})
set(DOXYGEN_PROJECT_ROOT "${OPACK_SOURCE_DIR}/")
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")

configure_file(${CMAKE_CURRENT_LIST_DIR}/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(${CMAKE_CURRENT_LIST_DIR}/conf.py ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

add_custom_target(
  GenerateDocs
  ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
  COMMAND python "${m.css_SOURCE_DIR}/documentation/doxygen.py" "${CMAKE_CURRENT_BINARY_DIR}/conf.py" --debug
  COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  SOURCES
    "conf.py"
    "Doxyfile"
    "pages/about.dox"
)

add_custom_target(
  OpenDocs 
  COMMAND "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
