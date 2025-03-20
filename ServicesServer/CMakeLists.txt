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
    BashRemoteService
    VsgViewerService
    hkDirService
    hkShellServerService
    Common
)

# Source files
set(SOURCES
    Common/hkServerServiceController.cpp
    Common/hkIServerServiceBase.cpp
    hkDirService/hkDirService.cpp
    hkShellServerService/hkShellServerService.cpp
    VsgViewerService/vsgViewerService.cpp
    BashRemoteService/bashProcess.cpp
    BashRemoteService/bashRemoteClient.cpp
    BashRemoteService/bashRemoteService.cpp
    BashRemoteService/bashRemoteClientTerminal.cpp
    BashRemoteService/processManager.cpp
    BashRemoteService/terminal.cpp
)

# Create an OBJECT library
set(LIB_NAME "${ROOT_PROJECT_NAME}ServicesServerObjects")
add_library(${LIB_NAME} OBJECT ${SOURCES})

