#pragma once

#include <memory>
#include <functional>
#include <string>

namespace alice2 {
namespace platform {

// Forward declarations
struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "Alice 2";
    bool resizable = true;
    bool fullscreen = false;
};

// Event types for unified event handling
enum class EventType {
    WindowClose,
    WindowResize,
    KeyPress,
    KeyRelease,
    MouseMove,
    MousePress,
    MouseRelease,
    MouseScroll
};

struct Event {
    EventType type;
    union {
        struct { int width, height; } resize;
        struct { int key, scancode, mods; } keyboard;
        struct { double x, y; } mouse;
        struct { int button, mods; } mouseButton;
        struct { double xOffset, yOffset; } scroll;
    } data;
};

using EventCallback = std::function<void(const Event&)>;

// Abstract platform interface for unified window and input management
class IPlatform {
public:
    virtual ~IPlatform() = default;
    
    // Core platform lifecycle
    virtual bool Initialize(const WindowConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual bool ShouldClose() = 0;
    virtual void PollEvents() = 0;
    virtual void SwapBuffers() = 0;
    
    // Window management
    virtual std::pair<int, int> GetFramebufferSize() = 0;
    virtual std::pair<int, int> GetWindowSize() = 0;
    virtual void SetWindowSize(int width, int height) = 0;
    virtual void* GetNativeWindow() = 0;
    
    // Event handling
    virtual void SetEventCallback(EventCallback callback) = 0;
    
    // Platform-specific surface creation for WebGPU
    virtual void* CreateWebGPUSurface(void* instance) = 0;
    
    // Platform identification
    virtual bool IsWeb() const = 0;
    virtual bool IsNative() const = 0;
    
    // Time utilities
    virtual double GetTime() = 0;
    
    // Input state queries (for immediate mode input)
    virtual bool IsKeyPressed(int key) = 0;
    virtual std::pair<double, double> GetMousePosition() = 0;
    virtual bool IsMouseButtonPressed(int button) = 0;
};

// Factory function for creating platform instances
std::unique_ptr<IPlatform> CreatePlatform();

// Platform-specific implementations will be in separate files:
// - native_platform.h/cpp (GLFW implementation)
// - web_platform.h/cpp (Emscripten implementation)

} // namespace platform
} // namespace alice2
