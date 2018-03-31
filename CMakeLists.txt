cmake_minimum_required(VERSION 3.6)
project(DRGraph)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Ofast -march=native -ffast-math -fopenmp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fopenmp")

# 3rd party
###############################################################################
# boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})
###############################################################################


###############################################################################

add_executable(DRGraph main.cpp data.cpp genrandom.cpp visualizemod.cpp visualizemod.h graph.cpp graph.h multilevel.cpp multilevel.h)
add_executable(Evaluation main2.cpp evaluation.cpp evaluation.h data.cpp graph.cpp graph.h)
target_link_libraries(DRGraph pthread gsl gslcblas ${Boost_LIBRARIES})
target_link_libraries(Evaluation pthread ${Boost_LIBRARIES})
###############################################################################

