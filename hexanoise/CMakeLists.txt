cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME hexanoise)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
set(WDIR "${CMAKE_CURRENT_SOURCE_DIR}/../win32")

find_package(BISON)
if(BISON_FOUND)
  BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
else()
  message("Bison not installed, using prebuilt parser.cpp")
  file(COPY "${WDIR}/parser.cpp" "${WDIR}/parser.hpp" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package(FLEX)
if(FLEX_FOUND)
  FLEX_TARGET(token tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
              COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/tokens.hpp)
  ADD_FLEX_BISON_DEPENDENCY(token parser)
else()
  message("Flex not installed, using prebuilt tokens.cpp")
  file(COPY "${WDIR}/tokens.cpp" "${WDIR}/tokens.hpp" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

find_package(PNG)
if(PNG_FOUND)
  add_definitions("-DHAVE_PNG")
  include_directories(${PNG_INCLUDE_DIRS} ${PNG_PNG_INCLUDE_DIR})
endif()

set(SOURCE_FILES
    analysis.cpp
    generator_context.cpp
    generator_opencl.cpp 
    generator_slowinterpreter.cpp
    node.cpp
    clew.c
    ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
)

set(HEADER_FILES
    ast.hpp
    analysis.hpp
    generator_context.hpp
    generator_i.hpp
    generator_opencl.hpp 
    clew.h 
    cl.hpp
    generator_slowinterpreter.hpp
    global_variables_i.hpp
    node.hpp
    simple_global_variables.hpp
    version.hpp)


find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIR})

set (LIBNAME_S "${LIBNAME}-s")
add_library(${LIBNAME_S} STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_library(${LIBNAME}   SHARED ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(${LIBNAME} PROPERTIES SOVERSION ${VERSION_SO} VERSION ${VERSION})
set_target_properties(${LIBNAME_S} PROPERTIES VERSION ${VERSION})

target_link_libraries(${LIBNAME} ${PNG_LIBRARIES})

if(BUILD_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_link_libraries(${LIBNAME}   -fprofile-arcs -ftest-coverage)
  target_link_libraries(${LIBNAME_S} -fprofile-arcs -ftest-coverage)
endif()

install(TARGETS ${LIBNAME_S} ${LIBNAME} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION "include/${LIBNAME}")
