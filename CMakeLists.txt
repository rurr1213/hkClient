
cmake_minimum_required(VERSION 3.10)

project(hkClient)

# the following prefixes all sub projects with this name, so that the
# sub project libraries are uniquely named, even if they are the same
# sub modules in other outer projects
set(ROOT_PROJECT_NAME ${PROJECT_NAME})

# Find required libraries (replace with your actual find_package commands)
find_package(Threads REQUIRED)

set(LIB_SOURCE_FILES
    hkClient.cpp
    hkClientMgr.cpp
    hkClientMgrExt.cpp
)

set(EXE_SOURCE_FILES
    hkClientLinux/hkClientAppLinux.cpp
)

add_subdirectory(genCppTools)
add_subdirectory(CommonCppDartCode)

add_library(hkClientLib STATIC ${LIB_SOURCE_FILES} $<TARGET_OBJECTS:${ROOT_PROJECT_NAME}genCppToolsObjects> $<TARGET_OBJECTS:${ROOT_PROJECT_NAME}CommonCppDartCodeObjects>)

# Include directories for the hkClient library
target_include_directories(hkClientLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    genCppTools
    CommonCppDartCode
    CommonCppDartCode/cpp
    CommonCppDartCode/cpp/json
    CommonCppDartCode/Messages
)

# Link libraries for the hkClient library
target_link_libraries(hkClientLib
    ${CMAKE_THREAD_LIBS_INIT}
)

# Set the library output directory
set_target_properties(hkClientLib PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    OUTPUT_NAME "hkClient"
)

#-------------------------------------------------------------------------------------#
# Create the hkClientAppLinux executable
add_executable(hkClientAppLinux ${EXE_SOURCE_FILES})

# Include directories for the hkClientAppLinux executable
target_include_directories(hkClientAppLinux PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    genCppTools
    CommonCppDartCode
    CommonCppDartCode/cpp
    CommonCppDartCode/cpp/json
    CommonCppDartCode/Messages
)

target_link_libraries(hkClientAppLinux
    hkClientLib
    ${CMAKE_THREAD_LIBS_INIT}
)

# Set the executable output directory
set_target_properties(hkClientAppLinux PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
