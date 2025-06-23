#pragma once

#include "platform_interface.h"

struct GLFWwindow;

namespace alice2 {
namespace platform {

class NativePlatform : public IPlatform {
public:
    NativePlatform();
    ~NativePlatform() override;

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
