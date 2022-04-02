set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# request to configure the file
configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

add_custom_target(opack_docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        COMMAND ${OPACK_SOURCE_DIR}/bin/docs/html/index.html
        WORKING_DIRECTORY "${OPACK_SOURCE_DIR}/include"
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM 
)
