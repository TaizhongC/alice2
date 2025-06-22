cmake_minimum_required(VERSION 3.0...3.25)
project(
    alice2 # name of the project, which will also be the name of the visual studio solution if you use it
    VERSION 0.1.0 # any version number
    LANGUAGES CXX C # programming languages used by the project
)

# Check if webgpu binaries exist, if not, download them
include(scripts/DownloadWebGPU.cmake)

# Include dependencies
if (NOT EMSCRIPTEN)
    add_subdirectory(glfw)
else()
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
endif()

add_subdirectory(depends/webgpu)

add_executable(alice2 src/main.cpp)

# Add the 'webgpu' target as a dependency of our App
target_link_libraries(alice2 PRIVATE webgpu)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it.
target_copy_webgpu_binaries(alice2)

set_target_properties(alice2 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

# Custom output directories for MSVC
if(MSVC)
    set_target_properties(alice2 PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/exe"
        PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/exe"
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

# Options that are specific to Emscripten
if (EMSCRIPTEN)
    # Enable the use of emscripten_sleep()
    target_link_options(alice2 PRIVATE -sASYNCIFY)

    # Generate a full web page rather than a simple WebAssembly module
    set_target_properties(alice2 PROPERTIES SUFFIX ".html")
endif()