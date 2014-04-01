cmake_minimum_required (VERSION 2.8.3)
set(EXE hndl2png)

include_directories(..)
link_directories(..)

add_executable(${EXE} hndl2png.cpp)

find_package(Boost ${REQUIRED_BOOST_VERSION} REQUIRED COMPONENTS chrono program_options filesystem)
find_package(PNG)

include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${EXE} ${Boost_LIBRARIES} ${PNG_LIBRARIES} OpenCL hexanoise)

# Installation
#install(TARGETS ${EXE} DESTINATION "${BINDIR}")
