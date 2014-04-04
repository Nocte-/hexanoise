project (unit-tests)
cmake_minimum_required (VERSION 2.8.3)
set(EXE unit_tests)

add_definitions(-DBOOST_TEST_DYN_LINK)

set(SOURCE_FILES "unit_tests.cpp")
add_executable(${EXE} ${SOURCE_FILES})

include_directories(..)

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS unit_test_framework)
find_package(OpenCL)
find_package(PNG)

include_directories(${Boost_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${PNG_PNG_INCLUDE_DIR})
target_link_libraries(${EXE} hexanoise ${Boost_LIBRARIES} ${PNG_LIBRARIES} OpenCL)
