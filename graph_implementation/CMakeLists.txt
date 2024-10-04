cmake_minimum_required(VERSION 3.10)
project(bgl_graph CXX)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify g++-12 as the compiler
if(APPLE)
    set(CMAKE_C_COMPILER "/opt/homebrew/Cellar/gcc/12.2.0/bin/gcc-12")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/Cellar/gcc/12.2.0/bin/g++-12")
    set(OpenMP_CXX_LIB_NAMES "gomp")
    set(OpenMP_gomp_LIBRARY "/opt/homebrew/Cellar/gcc/12.2.0/lib/gcc/12/libgomp.dylib")
elseif(UNIX AND NOT APPLE)
    set(CMAKE_C_COMPILER "/usr/bin/gcc-12")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++-12")
endif()

# Open MP
set(OpenMP_CXX_FLAGS "-fopenmp")

find_package(OpenMP REQUIRED COMPONENTS CXX)

if(OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# Include test directory
add_subdirectory(test)

# Set the path to the Boost installation
set(BOOST_ROOT "/home/ziming.zeng/boost_1_85_0")
set(BOOST_INCLUDE_DIR "/home/ziming.zeng/boost_1_85_0")
set(BOOST_LIBRARYDIR "/home/ziming.zeng/boost_1_85_0/stage/lib")

# Find and include Boost
find_package(Boost 1.85 REQUIRED)
include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARYDIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
# Make uni_graph.cpp a library
add_library(graph_bgl STATIC src/uni_graph.cpp)
target_link_libraries(graph_bgl PRIVATE OpenMP::OpenMP_CXX ${Boost_LIBRARIES})
# Use the library in main.cpp
add_executable(uni_graph src/main.cpp)
target_link_libraries(uni_graph PRIVATE graph_bgl)

