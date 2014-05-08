project(hexanoise)
cmake_minimum_required(VERSION 2.8.3)

set(VERSION_SO    "1")
set(VERSION_MAJOR "0")
set(VERSION_MINOR "1")
set(VERSION_PATCH "3")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CPACK_PACKAGE_NAME "lib${CMAKE_PROJECT_NAME}${VERSION_SO}")
set(WEBPAGE "http://github.com/Nocte-/hexanoise")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(BUILD_UNITTESTS 0 CACHE BOOL "Build the unit tests")
set(BUILD_COVERAGE  0 CACHE BOOL "Code coverage (gcc only)")
set(BUILD_UTILITIES 0 CACHE BOOL "Build utilities (hndl2png)")
set(BUILD_DOCUMENTATION 0 CACHE BOOL "Generate Doxygen documentation")

set(REQUIRED_BOOST_VERSION 1.48)

# Set up the compiler
#
if(MSVC)
    add_definitions(/D_WIN32_WINNT=0x0501 /D_CRT_SECURE_NO_WARNINGS /DYY_NO_UNISTD_H)
    set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} /MP /EHsc /wd4244 /wd4996 ")
    set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /MP /GS- /Ox /Ob2 /Oi /Oy /arch:SSE /fp:fast /Zi")
    set(CMAKE_LIB_LINKER_FLAGS  "${CMAKE_LIB_LINKER_FLAGS} /OPT:REF /SUBSYSTEM:WINDOWS")

else() # Most likely gcc or clang
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif ("${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "4.7")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()

    if (BUILD_COVERAGE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG   "-g")
    set(CMAKE_LIB_LINKER_FLAGS  "-s")
endif()

add_subdirectory(hexanoise)

if(BUILD_UNITTESTS)
  add_subdirectory(unit_tests)
endif()
if(BUILD_UTILITIES)
  add_subdirectory(util)
endif()

# Doxygen documentation
#
if(BUILD_DOCUMENTATION)
  find_package(Doxygen)
  if (DOXYGEN_FOUND STREQUAL "YES")
      configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES api-doc)
      get_target_property(DOC_TARGET doc TYPE)
      if (NOT DOC_TARGET)
          add_custom_target(doc)
      endif()
      add_dependencies(doc doxygen)
  endif()
endif()

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Coherent noise library")
set(CPACK_PACKAGE_VENDOR "Nocte")
set(CPACK_PACKAGE_CONTACT "Nocte <nocte@hippie.nu>")
set(CPACK_SOURCE_IGNORE_FILES "CMakefiles;Makefile;CMakeCache.txt;_CPack_Packages;/.git/;.gitignore;.travis.yml")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${VERSION}")

if(UNIX)
    set(CPACK_GENERATOR "DEB")
    add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

    set(PKGCONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/install/hexanoise.pc)
    configure_file(${PKGCONFIG_FILE}.in ${PKGCONFIG_FILE})
    install(FILES ${PKGCONFIG_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/install/FindHexanoise.cmake DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules)

    set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
    add_custom_target(deb dpkg-buildpackage)
    add_dependencies(deb dist)

elseif(WIN32)
    set(CPACK_GENERATOR "ZIP")

endif()

include(CPack)

