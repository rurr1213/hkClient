cmake_minimum_required(VERSION 3.10)

project(genCppTools)

set(CMAKE_CXX_STANDARD 20)

# Add include directories
include_directories(
    ..
    ../CommonCppDartCode/Messages
    ../CommonCppDartCode/cpp
    ../CommonCppDartCode/cpp/json
    .
    ../genCppTools
    ../..
)

# Source files
set(SOURCES
    hkClientServiceController.cpp
)

# Create an OBJECT library
set(LIB_NAME "${ROOT_PROJECT_NAME}ServicesClientObjects")
add_library(${LIB_NAME} OBJECT ${SOURCES})

