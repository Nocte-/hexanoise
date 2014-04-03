cmake_minimum_required (VERSION 2.8.3)
set(EXE hndl2png)

include_directories(..)
link_directories(..)

add_executable(${EXE} hndl2png.cpp)

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS program_options)
find_package(PNG)
find_package(OpenCL)

include_directories(${Boost_INCLUDE_DIRS} ${OPENCL_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${PNG_PNG_INCLUDE_DIR})
target_link_libraries(${EXE} ${Boost_LIBRARIES} ${PNG_LIBRARIES} ${OPENCL_LIBRARIES} hexanoise-s)

# Installation
#install(TARGETS ${EXE} DESTINATION "${BINDIR}")
