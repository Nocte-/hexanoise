cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME hexanoise)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_BINARY_DIR})

find_package(BISON REQUIRED)
BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

find_package(FLEX REQUIRED)
FLEX_TARGET(token tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
            COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/tokens.hpp)
ADD_FLEX_BISON_DEPENDENCY(token parser)

set(SOURCE_FILES
    generator_context.cpp
    generator_opencl.cpp
    generator_slowinterpreter.cpp
    node.cpp
    ${FLEX_token_OUTPUTS}
    ${BISON_parser_OUTPUTS}
)

set(HEADER_FILES
    ast.hpp
    generator_context.hpp
    generator_i.hpp
    generator_opencl.hpp
    generator_slowinterpreter.hpp
    node.hpp
    parser.hpp
    tokens.hpp
    version.hpp)

set (LIBNAME_S "${LIBNAME}-s")
add_library(${LIBNAME_S} STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_library(${LIBNAME}   SHARED ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(${LIBNAME} PROPERTIES SOVERSION ${VERSION_SO} VERSION ${VERSION})
set_target_properties(${LIBNAME_S} PROPERTIES VERSION ${VERSION})

install(TARGETS ${LIBNAME_S} ${LIBNAME} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION "include/${LIBNAME}")

