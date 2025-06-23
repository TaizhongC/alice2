cmake_minimum_required(VERSION 3.10)

# Unified Alice 2 Project with CODA Web Integration
project(alice2_unified VERSION 1.0.0 LANGUAGES CXX C)

# Compiler and CMake settings
cmake_policy(SET CMP0135 NEW)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)

# Project settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform detection and configuration
if (EMSCRIPTEN)
    message(STATUS "Configuring Alice 2 for Emscripten web deployment...")
    set(ALICE2_PLATFORM "WEB")
    
    # Web-specific sources
    set(ALICE2_PLATFORM_SOURCES
        src/platform/web_platform.cpp
        src/platform/web/web_events.cpp
        src/platform/web/javascript_interface.cpp
    )
    
    # Emscripten-specific flags
    set(EMSCRIPTEN_FLAGS
        "-std=c++20"
        "-DALICE2_WEB_PLATFORM"
        "-s USE_GLFW=3"
        "-s USE_WEBGPU=1"
        "-s WASM=1"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s INITIAL_MEMORY=67108864"
        "-s MAXIMUM_MEMORY=268435456"
        "-s NO_EXIT_RUNTIME=1"
        "-s DISABLE_EXCEPTION_CATCHING=0"
        "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "-s EXPORTED_FUNCTIONS=['_main','_malloc','_free']"
        "-lembind"
        "--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/web/shell.html"
    )
    
else()
    message(STATUS "Configuring Alice 2 for native deployment...")
    set(ALICE2_PLATFORM "NATIVE")
    
    # Native-specific sources
    set(ALICE2_PLATFORM_SOURCES
        src/platform/native_platform.cpp
    )
    
    # Include WebGPU download script for native builds
    include(scripts/DownloadWebGPU.cmake)
endif()

# Core source files (platform-agnostic)
set(ALICE2_CORE_SOURCES
    src/app/unified_application.cpp
    src/app/scene.cpp
    src/app/camera.cpp
    src/renderer/unified_renderer.cpp
    src/platform/platform_factory.cpp
)

# Legacy CODA sources (to be gradually migrated)
# Note: CODA sources are currently disabled until they are implemented
set(CODA_LEGACY_SOURCES
    # src/coda/core/geometry/Graph.cpp
    # src/coda/core/geometry/Mesh.cpp
    # src/coda/core/utilities/Math.cpp
    # src/coda/core/interface/functionset/FnGraph.cpp
    # src/coda/core/interface/functionset/FnMesh.cpp
    # src/coda/core/interface/objects/ObjGraph.cpp
    # src/coda/core/interface/objects/ObjMesh.cpp
    # src/coda/core/interface/iterators/ItGraph.cpp
    # src/coda/core/interface/iterators/ItMesh.cpp
)

# Combine all sources
set(ALICE2_ALL_SOURCES 
    ${ALICE2_CORE_SOURCES} 
    ${ALICE2_PLATFORM_SOURCES}
    ${CODA_LEGACY_SOURCES}
    src/main_unified.cpp
)

# Dependencies
if (NOT EMSCRIPTEN)
    # Native dependencies
    add_subdirectory(depends/glfw)
    add_subdirectory(depends/glfw3webgpu)
    add_subdirectory(depends/webgpu)
else()
    # Web dependencies (Emscripten provides these)
    add_library(glfw INTERFACE)
    target_link_options(glfw INTERFACE -sUSE_GLFW=3)
    
    add_library(webgpu INTERFACE)
    target_link_options(webgpu INTERFACE -sUSE_WEBGPU=1)
    target_include_directories(webgpu INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/depends/webgpu/include-emscripten
    )
endif()

# Legacy dependencies
# Note: nlohmann JSON dependency disabled until CODA sources are implemented
# add_subdirectory(src/coda/core/depends/nlohmann)

# Create the executable
add_executable(alice2_unified ${ALICE2_ALL_SOURCES})

# Include directories
target_include_directories(alice2_unified PRIVATE 
    src
    src/coda
    src/coda/core
    src/platform
    src/renderer
    src/app
)

# Platform-specific linking and configuration
if (EMSCRIPTEN)
    # Web build configuration
    target_link_libraries(alice2_unified PRIVATE
        glfw
        webgpu
        # nlohmann_json::nlohmann_json  # Disabled until CODA sources are implemented
    )
    
    # Apply Emscripten flags
    target_compile_options(alice2_unified PRIVATE ${EMSCRIPTEN_FLAGS})
    target_link_options(alice2_unified PRIVATE ${EMSCRIPTEN_FLAGS})
    
    # Set output properties for web
    set_target_properties(alice2_unified PROPERTIES
        OUTPUT_NAME "alice2_web"
        SUFFIX ".html"
    )
    
    # Copy web assets
    set(WEB_ASSETS
        src/platform/web/shell.html
        src/platform/web/ui/controls.js
        src/platform/web/ui/styles.css
    )
    
    foreach(ASSET ${WEB_ASSETS})
        configure_file(
            ${CMAKE_SOURCE_DIR}/${ASSET}
            ${CMAKE_BINARY_DIR}/bin/${ASSET}
            COPYONLY
        )
    endforeach()
    
else()
    # Native build configuration
    target_link_libraries(alice2_unified PRIVATE
        webgpu
        glfw
        glfw3webgpu
        # nlohmann_json::nlohmann_json  # Disabled until CODA sources are implemented
    )
    
    # Copy WebGPU binaries for native builds
    target_copy_webgpu_binaries(alice2_unified)
    
    # Set warning levels
    if (MSVC)
        target_compile_options(alice2_unified PRIVATE /W4)
    else()
        target_compile_options(alice2_unified PRIVATE -Wall -Wextra -pedantic)
    endif()
    
    # MSVC-specific output directories
    if(MSVC)
        set_target_properties(alice2_unified PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/exe"
            PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_VS_PLATFORM_NAME}/$<CONFIG>/exe"
        )
    endif()
    
    # XCode specific settings
    if (XCODE)
        set_target_properties(alice2_unified PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal"
        )
    endif()
endif()

# Compiler definitions
target_compile_definitions(alice2_unified PRIVATE
    ALICE2_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    ALICE2_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    ALICE2_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# Platform-specific definitions
if (EMSCRIPTEN)
    target_compile_definitions(alice2_unified PRIVATE 
        ALICE2_WEB_PLATFORM
        USE_EMSCRIPTEN
    )
else()
    target_compile_definitions(alice2_unified PRIVATE 
        ALICE2_NATIVE_PLATFORM
    )
endif()

# Build information
message(STATUS "Alice 2 Unified Build Configuration:")
message(STATUS "  Platform: ${ALICE2_PLATFORM}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
if (EMSCRIPTEN)
    message(STATUS "  Emscripten: ${EMSCRIPTEN_VERSION}")
    message(STATUS "  Output: alice2_web.html")
else()
    message(STATUS "  Output: alice2_unified")
endif()
