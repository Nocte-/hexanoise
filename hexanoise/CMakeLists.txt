cmake_minimum_required (VERSION 2.8.3)
set(LIBNAME hexanoise)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
               ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_BINARY_DIR})

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(BISON)
if(BISON_FOUND)
  BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)
endif()

find_package(FLEX)
if(FLEX_FOUND)
  FLEX_TARGET(token tokens.l ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
              COMPILE_FLAGS --header-file=${CMAKE_CURRENT_BINARY_DIR}/tokens.hpp)
  ADD_FLEX_BISON_DEPENDENCY(token parser)
endif()

find_package(OpenCL)
if(OPENCL_FOUND)
  add_definitions("-DHAVE_OPENCL")
  include_directories(${OPENCL_INCLUDE_DIRS})
endif()

find_package(PNG)
if(PNG_FOUND)
  add_definitions("-DHAVE_PNG")
  include_directories(${PNG_INCLUDE_DIRS} ${PNG_PNG_INCLUDE_DIR})
endif()

set(SOURCE_FILES
    analysis.cpp
    generator_context.cpp
    generator_slowinterpreter.cpp
    node.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/tokens.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
)

set(HEADER_FILES
    ast.hpp
    analysis.hpp
    generator_context.hpp
    generator_i.hpp
    generator_slowinterpreter.hpp
    node.hpp
    version.hpp)

if(OPENCL_FOUND)
  set(SOURCE_FILES ${SOURCE_FILES} generator_opencl.cpp)
  set(HEADER_FILES ${HEADER_FILES} generator_opencl.hpp)
endif()

include_directories(${GLM_INCLUDEDIR})

set (LIBNAME_S "${LIBNAME}-s")
add_library(${LIBNAME_S} STATIC ${SOURCE_FILES} ${HEADER_FILES})
add_library(${LIBNAME}   SHARED ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(${LIBNAME} PROPERTIES SOVERSION ${VERSION_SO} VERSION ${VERSION})
set_target_properties(${LIBNAME_S} PROPERTIES VERSION ${VERSION})

target_link_libraries(${LIBNAME} ${PNG_LIBRARIES} ${OPENCL_LIBRARIES})

install(TARGETS ${LIBNAME_S} ${LIBNAME} DESTINATION lib)
install(FILES ${HEADER_FILES} DESTINATION "include/${LIBNAME}")
