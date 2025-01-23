# /home/ubuntu/dev/hk/src/backChannelClient/CMakeLists.txt

#add_subdirectory(backChannelClientLinux)
#add_subdirectory(backChannelClientWin) # Assuming you'll have a similar structure for Windows


# /home/ubuntu/dev/hk/src/backChannelClient/backChannelClientLinux/CMakeLists.txt

# Find required libraries (replace with your actual find_package commands)
find_package(Threads REQUIRED)

set(TARGET_NAME hkClientAppLinux)

# Example: If using a library installed with CMake
# find_package(nlohmann_json REQUIRED)


# Define source files
set(SOURCE_FILES
    hkClientLinux/hkClientAppLinux.cpp
    backChannelClient.cpp
    hyperCubeClient.cpp
    hkClientMgr.cpp
)

add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Include directories (adapt paths as necessary)
target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src/genCppTools  # Assuming genCppTools is in hk/src/tools
    ${PROJECT_SOURCE_DIR}/src/CommonCppDartCode # Assuming CommonCppDartCode is in hk/src
    ${PROJECT_SOURCE_DIR}/src/CommonCppDartCode/cpp
    ${PROJECT_SOURCE_DIR}/src/CommonCppDartCode/cpp/json
    ${PROJECT_SOURCE_DIR}/src/CommonCppDartCode/Messages
    ${PROJECT_SOURCE_DIR}/src/backChannelClient
    # For external libraries included without CMake
    # /path/to/json  # Include path for the json library
)


# Link libraries (replace with the names of your libraries)
target_link_libraries(${TARGET_NAME}
    genCppTools
    CommonCppDartCode
    ${CMAKE_THREAD_LIBS_INIT} # For threading support
    # Link with your generated tools library
    # nlohmann_json::nlohmann_json # Example for a CMake-installed library
    # json # If using find_library for json
    # Other libraries: CommonCppDartCode (and perhaps Messages separately?)
)

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# /home/ubuntu/dev/hk/src/backChannelClient/backChannelClientWin/CMakeLists.txt
# (This will be very similar to the Linux version, with potential adjustments for Windows-specific libraries/dependencies)

# ... (Content similar to backChannelClientLinux CMakeLists.txt, adapted for Windows) ...

