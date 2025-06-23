# Alice 2 - Unified Dual-Deployment System

A modern cross-platform application framework that combines Alice 2's WebGPU rendering capabilities with CODA's web deployment system, creating a unified dual-deployment solution for both native desktop and web platforms.

## 🚀 Key Features

### Unified Architecture
- **Single Codebase**: One source tree for both native and web deployment
- **WebGPU Everywhere**: Modern graphics API across all platforms
- **Platform Abstraction**: Unified interface for GLFW (native) and Emscripten (web)
- **Modern C++20**: RAII, smart pointers, and modern language features

### Dual Deployment
- **Native Desktop**: High-performance native applications using GLFW + WebGPU
- **Web Browser**: Full-featured web applications using Emscripten + WebGPU
- **Feature Parity**: Consistent functionality across both platforms
- **Automatic Build System**: Seamless builds for both targets

### Rich Web Integration
- **Interactive UI**: Modern web controls with real-time parameter adjustment
- **JavaScript Bridge**: Seamless C++ ↔ JavaScript communication
- **Responsive Design**: Adaptive layout for different screen sizes
- **Performance Monitoring**: Built-in FPS counter and performance metrics

### CODA Compatibility
- **Immediate Mode API**: Familiar drawing functions (DrawPoint, DrawLine, etc.)
- **Scene Management**: Compatible object and scene system
- **Camera Controls**: Integrated camera system with user interaction
- **Asset Pipeline**: Unified resource management

## 🏗️ Architecture Overview

```
Alice 2 Unified System
├── Platform Abstraction Layer
│   ├── IPlatform (interface)
│   ├── NativePlatform (GLFW implementation)
│   └── WebPlatform (Emscripten implementation)
├── Unified Renderer (WebGPU)
│   ├── Immediate Mode API
│   ├── Batch Rendering
│   └── Cross-platform Shaders
├── Application Framework
│   ├── UnifiedApplication
│   ├── Scene Management
│   └── Camera System
└── Build System
    ├── CMake Configuration
    ├── Dependency Management
    └── Asset Pipeline
```

## 📋 Prerequisites

### For Native Builds
- **CMake** 3.10 or higher
- **C++20 compatible compiler** (GCC 10+, Clang 12+, MSVC 2019+)
- **Git** (for dependency management)

### For Web Builds
- **Emscripten SDK** (latest version recommended)
- All native build prerequisites

### Platform-Specific
- **Windows**: Visual Studio 2019+ or MinGW-w64
- **macOS**: Xcode 12+ or Clang
- **Linux**: GCC 10+ or Clang 12+

## 🔧 Quick Start

### 1. Clone and Setup
```bash
git clone <repository-url>
cd alice2_unified
```

### 2. Build Native Version
```powershell
# Windows PowerShell
.\build_unified.ps1 -Target native

# Or manually
mkdir build_native
cmake -S . -B build_native -f CMakeLists_unified.txt
cmake --build build_native
```

### 3. Build Web Version
```powershell
# Ensure Emscripten is activated
source path/to/emsdk/emsdk_env.sh  # Linux/macOS
# or
emsdk_env.bat  # Windows

# Build web version
.\build_unified.ps1 -Target web

# Or manually
mkdir build_web
emcmake cmake -S . -B build_web -f CMakeLists_unified.txt
cmake --build build_web
```

### 4. Run Applications

**Native:**
```bash
./build_native/alice2_unified
```

**Web:**
```bash
# Start a local server
cd _output/web
python -m http.server 8000
# Open http://localhost:8000/alice2_web.html
```

## 🛠️ Build System

### Build Script Options
```powershell
.\build_unified.ps1 [options]

Options:
  -Target <target>   # "native", "web", or "both"
  -Command <cmd>     # "build", "clean", or "rebuild"
  -Config <config>   # "Debug" or "Release"
  -NoWarning         # Suppress CMake warnings

Examples:
  .\build_unified.ps1                    # Build native version
  .\build_unified.ps1 -Target web        # Build web version
  .\build_unified.ps1 -Target both       # Build both versions
  .\build_unified.ps1 -Command clean     # Clean all builds
  .\build_unified.ps1 -Command rebuild   # Clean and rebuild
```

### Manual CMake Configuration
```bash
# Native build with specific backend
cmake -B build -DWEBGPU_BACKEND=DAWN -DCMAKE_BUILD_TYPE=Release

# Web build
emcmake cmake -B build_web -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

## 💻 Development Guide

### Project Structure
```
alice2_unified/
├── src/
│   ├── app/                    # Application framework
│   │   ├── unified_application.h/cpp
│   │   ├── scene.cpp
│   │   └── camera.cpp
│   ├── platform/               # Platform abstraction
│   │   ├── platform_interface.h
│   │   ├── native_platform.h/cpp
│   │   ├── web_platform.h/cpp
│   │   └── web/                # Web-specific assets
│   │       ├── shell.html
│   │       └── ui/
│   ├── renderer/               # Unified WebGPU renderer
│   │   └── unified_renderer.h/cpp
│   ├── coda/                   # Legacy CODA components
│   │   └── core/
│   └── main_unified.cpp        # Application entry point
├── depends/                    # Third-party dependencies
│   ├── glfw/
│   ├── webgpu/
│   └── glfw3webgpu/
├── scripts/                    # Build scripts
│   └── DownloadWebGPU.cmake
├── CMakeLists_unified.txt      # Unified build configuration
├── build_unified.ps1           # Build automation script
└── README_unified.md           # This file
```

### Adding New Features

1. **Platform-Agnostic Code**: Add to `src/app/` or `src/renderer/`
2. **Platform-Specific Code**: Add to appropriate platform implementation
3. **Web UI Controls**: Modify `src/platform/web/ui/controls.js`
4. **Shaders**: Add to `src/renderer/shaders/`

### API Usage Examples

```cpp
// Basic rendering
auto* renderer = app.GetRenderer();
renderer->BeginFrame();
renderer->DrawPoint(Vec3f(0, 0, 0), Color::Red(), 5.0f);
renderer->DrawLine(Vec3f(-1, 0, 0), Vec3f(1, 0, 0), Color::Blue());
renderer->EndFrame();

// Batch rendering for performance
renderer->BeginPoints();
for (const auto& point : points) {
    renderer->AddPoint(point.position, point.color, point.size);
}
renderer->EndPoints();

// Event handling
void OnEvent(const platform::Event& event) override {
    if (event.type == platform::EventType::KeyPress) {
        switch (event.data.keyboard.key) {
            case 'R': ResetScene(); break;
            case 'F': ToggleFullscreen(); break;
        }
    }
}
```

## 🎯 Migration Strategy

### From CODA
1. **Immediate**: Use existing CODA geometry and scene code
2. **Phase 1**: Replace WebGL renderer with WebGPU
3. **Phase 2**: Modernize C++ code (C++11 → C++20)
4. **Phase 3**: Optimize for performance and features

### From Alice 2
1. **Immediate**: Use existing WebGPU renderer and build system
2. **Phase 1**: Add web platform support
3. **Phase 2**: Integrate CODA's web UI and features
4. **Phase 3**: Unify APIs and optimize

## 🚀 Deployment

### Native Distribution
- **Windows**: Standalone executable with WebGPU DLLs
- **macOS**: App bundle with embedded frameworks
- **Linux**: AppImage or system packages

### Web Distribution
- **Static Hosting**: Deploy to any web server
- **CDN**: Optimize for global distribution
- **Progressive Web App**: Add PWA manifest for app-like experience

## 🔍 Troubleshooting

### Common Issues

**WebGPU not available:**
- Ensure browser supports WebGPU (Chrome 113+, Firefox with flag)
- Check console for WebGPU initialization errors

**Build failures:**
- Verify all prerequisites are installed
- Check Emscripten activation for web builds
- Ensure CMake can find dependencies

**Performance issues:**
- Enable Release build configuration
- Check browser developer tools for bottlenecks
- Monitor FPS counter in web UI

### Debug Mode
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug

# Web debugging
emcmake cmake -DCMAKE_BUILD_TYPE=Debug -DEMSCRIPTEN_GENERATE_MAP=1
```

## 📈 Performance Considerations

- **Batch Rendering**: Use batch APIs for multiple objects
- **Memory Management**: Leverage RAII and smart pointers
- **WebGPU Optimization**: Minimize state changes and draw calls
- **Web Performance**: Optimize WASM size and loading

## 🤝 Contributing

1. Follow the existing code style and architecture
2. Test both native and web builds
3. Update documentation for new features
4. Add unit tests where appropriate

## 📄 License

[Specify your license here]

---

This unified system represents the best of both Alice 2 and CODA projects, providing a modern, cross-platform foundation for graphics applications with seamless dual deployment capabilities.
