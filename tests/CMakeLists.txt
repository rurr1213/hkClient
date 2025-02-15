cmake_minimum_required(VERSION 3.10)
project(GroupActivityDataTests)

set(CMAKE_CXX_STANDARD 14)

# Add Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/src/hkClient)
include_directories(${CMAKE_SOURCE_DIR}/src/hkClient/genCppTools)

# Add the source files
add_executable(test_groupActivityData test_groupActivityData.cpp)
target_link_libraries(test_groupActivityData ${GTEST_LIBRARIES} pthread)

# Set the output directory for the test executable
set_target_properties(test_groupActivityData PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests/hkClient/tests
)