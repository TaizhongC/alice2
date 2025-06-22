cmake_minimum_required(VERSION 3.0...3.25)
project(
    alice2 # name of the project, which will also be the name of the visual studio solution if you use it
    VERSION 0.1.0 # any version number
    LANGUAGES CXX C # programming languages used by the project
)

add_executable(alice2 src/main.cpp)

set_target_properties(alice2 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

# Custom output directories for MSVC
if(MSVC)
    set_target_properties(alice2 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/EXE"
        PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/EXE"
    )
endif()

# Set warning levels
if (MSVC)
    target_compile_options(alice2 PRIVATE /W4)
else()
    target_compile_options(alice2 PRIVATE -Wall -Wextra -pedantic)
endif()

# XCode specific settings
if (XCODE)
    set_target_properties(alice2 PROPERTIES
        XCODE_GENERATE_SCHEME ON
        XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
    )
endif()