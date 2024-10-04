cmake_minimum_required(VERSION 3.10)

# Set the path to the directories containing the headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Add test executable
add_executable(runTests test_main.cpp test_uni_graph.cpp)

# Link test executable with needed libraries
target_link_libraries(runTests PRIVATE graph_bgl ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} OpenMP::OpenMP_CXX)