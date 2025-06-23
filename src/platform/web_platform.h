#pragma once

#include "platform_interface.h"

struct GLFWwindow;

namespace alice2 {
namespace platform {

class WebPlatform : public IPlatform {
public:
    WebPlatform();
    ~WebPlatform() override;

    // Core platform lifecycle
    bool Initialize(const WindowConfig& config) override;
    void Shutdown() override;
    bool ShouldClose() override;
    void PollEvents() override;
    void SwapBuffers() override;

    // Window management
    std::pair<int, int> GetFramebufferSize() override;
    std::pair<int, int> GetWindowSize() override;
    void SetWindowSize(int width, int height) override;
    void* GetNativeWindow() override;

    // Event handling
    void SetEventCallback(EventCallback callback) override;

    // Platform-specific surface creation for WebGPU
    void* CreateWebGPUSurface(void* instance) override;

    // Platform identification
    bool IsWeb() const override;
    bool IsNative() const override;

    // Time utilities
    double GetTime() override;

    // Input state queries
    bool IsKeyPressed(int key) override;
    std::pair<double, double> GetMousePosition() override;
    bool IsMouseButtonPressed(int button) override;

private:
    GLFWwindow* m_Window;
    EventCallback m_EventCallback;
};

} // namespace platform
} // namespace alice2

#include "platform_interface.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/html5.h>

namespace alice2 {
namespace platform {

class WebPlatform : public IPlatform {
public:
    WebPlatform();
    ~WebPlatform() override;
    
    // IPlatform implementation
    bool Initialize(const WindowConfig& config) override;
    void Shutdown() override;
    bool ShouldClose() override;
    void PollEvents() override;
    void SwapBuffers() override;
    
    std::pair<int, int> GetFramebufferSize() override;
    std::pair<int, int> GetWindowSize() override;
    void SetWindowSize(int width, int height) override;
    void* GetNativeWindow() override;
    
    void SetEventCallback(EventCallback callback) override;
    void* CreateWebGPUSurface(void* instance) override;
    
    bool IsWeb() const override { return true; }
    bool IsNative() const override { return false; }
    
    double GetTime() override;
    
    bool IsKeyPressed(int key) override;
    std::pair<double, double> GetMousePosition() override;
    bool IsMouseButtonPressed(int button) override;
    
    // Web-specific functionality
    void SetMainLoop(em_callback_func func, void* userData, int fps = 0);
    void RegisterJavaScriptCallbacks();

private:
    EventCallback m_EventCallback;
    std::string m_CanvasId = "canvas";
    int m_Width = 800;
    int m_Height = 600;
    bool m_ShouldClose = false;
    
    // Canvas and context management
    bool SetupCanvas();
    void UpdateCanvasSize();
    
    // Event handling
    static EM_BOOL KeyCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData);
    static EM_BOOL MouseCallback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData);
    static EM_BOOL WheelCallback(int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData);
    static EM_BOOL ResizeCallback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData);
    
    // JavaScript integration for CODA-style UI controls
    void SetupUICallbacks();
    
    // Key and mouse state tracking
    bool m_KeyStates[512] = {false}; // Track key states
    bool m_MouseButtonStates[8] = {false}; // Track mouse button states
    double m_MouseX = 0.0, m_MouseY = 0.0;
    
    WebPlatform* GetPlatformFromUserData(void* userData);
};

} // namespace platform
} // namespace alice2

#endif // __EMSCRIPTEN__
