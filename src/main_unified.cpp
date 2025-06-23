#include "app/unified_application.h"
#include "app/scene.h"
#include "app/camera.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>

// Web-specific main loop
void web_main_loop() {
    alice2::UnifiedApplication::Get().MainLoop();
}

// JavaScript interface functions
extern "C" {
    EMSCRIPTEN_KEEPALIVE void alice2_set_background_brightness(float brightness) {
        alice2::UnifiedApplication::Get().SetBackgroundBrightness(brightness);
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_add_test_geometry() {
        alice2::UnifiedApplication::Get().AddTestGeometry();
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_clear_scene() {
        if (auto* scene = alice2::UnifiedApplication::Get().GetScene()) {
            scene->Clear();
        }
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_resize(int width, int height) {
        alice2::UnifiedApplication::Get().Resize(width, height);
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_reset_camera() {
        // Reset camera to default position
        if (auto* scene = alice2::UnifiedApplication::Get().GetScene()) {
            if (auto* camera = scene->GetCamera()) {
                camera->SetPosition(alice2::Vec3f(0, 0, 5));
                camera->SetTarget(alice2::Vec3f(0, 0, 0));
            }
        }
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_set_point_size(float size) {
        // Implementation would set global point size
        std::cout << "Setting point size to: " << size << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_set_line_width(float width) {
        // Implementation would set global line width
        std::cout << "Setting line width to: " << width << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_set_fov(float fov) {
        // Implementation would set camera FOV
        std::cout << "Setting FOV to: " << fov << " degrees" << std::endl;
    }
    
    EMSCRIPTEN_KEEPALIVE void alice2_toggle_wireframe() {
        // Implementation would toggle wireframe mode
        std::cout << "Toggling wireframe mode" << std::endl;
    }
}

#endif

int main() {
    std::cout << "Starting Alice 2 Unified Application..." << std::endl;
    
    // Create application instance
    auto& app = alice2::UnifiedApplication::Get();
    
    // Configure window
    alice2::platform::WindowConfig config;
    config.width = 1200;
    config.height = 800;
    config.title = "Alice 2 - Unified WebGPU Application";
    config.resizable = true;
    
    // Initialize application
    if (!app.Initialize(config)) {
        std::cerr << "Failed to initialize Alice 2 application" << std::endl;
        return 1;
    }
    
    std::cout << "Alice 2 application initialized successfully" << std::endl;
    
#ifdef __EMSCRIPTEN__
    // Web platform: Set up main loop with Emscripten
    std::cout << "Setting up web main loop..." << std::endl;
    emscripten_set_main_loop(web_main_loop, 0, true);
#else
    // Native platform: Run traditional main loop
    std::cout << "Running native main loop..." << std::endl;
    app.Run();
#endif
    
    // Cleanup (only reached on native platform)
    app.Shutdown();
    std::cout << "Alice 2 application shutdown complete" << std::endl;
    
    return 0;
}

// Example of how to extend the application
class CustomAlice2App : public alice2::UnifiedApplication {
public:
    void OnUpdate(float deltaTime) override {
        // Custom update logic
        UnifiedApplication::OnUpdate(deltaTime);
        
        // Add custom behavior here
        // For example: physics simulation, AI updates, etc.
    }
    
    void OnRender() override {
        // Custom rendering logic
        UnifiedApplication::OnRender();
        
        // Add custom rendering here
        // For example: custom shaders, post-processing, etc.
    }
    
    void OnEvent(const alice2::platform::Event& event) override {
        // Custom event handling
        UnifiedApplication::OnEvent(event);
        
        // Handle custom events here
        if (event.type == alice2::platform::EventType::KeyPress) {
            // Custom key handling
            switch (event.data.keyboard.key) {
                case 'R': // Reset scene
                    GetScene()->Clear();
                    AddTestGeometry();
                    break;
                case 'F': // Toggle fullscreen (native only)
                    #ifndef __EMSCRIPTEN__
                    // Fullscreen toggle implementation
                    #endif
                    break;
            }
        }
    }
};

/*
Usage Examples:

1. Native Build:
   mkdir build_native
   cmake -S . -B build_native -f CMakeLists_unified.txt
   cmake --build build_native
   ./build_native/alice2_unified

2. Web Build:
   mkdir build_web
   emcmake cmake -S . -B build_web -f CMakeLists_unified.txt
   cmake --build build_web
   # Serve build_web/bin/alice2_web.html

3. Using the build script:
   ./build_unified.ps1 -Target both
   
4. Development workflow:
   # Make changes to source
   ./build_unified.ps1 -Target web -Command rebuild
   # Test in browser
   
   ./build_unified.ps1 -Target native -Command rebuild
   # Test native version

Key Features of this Unified System:
- Single codebase for both native and web deployment
- WebGPU rendering on both platforms
- CODA-compatible immediate mode rendering API
- Rich web UI with JavaScript integration
- Modern C++20 architecture with RAII
- Automatic WebGPU binary management
- Cross-platform input and event handling
- Extensible application framework
*/
