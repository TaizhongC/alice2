#pragma once

#include <memory>
#include <functional>
#include <vector>
#include "../platform/platform_interface.h"
#include "../renderer/unified_renderer.h"
#include "../core/base/Types.h"

namespace alice2 {

// Forward declarations
class Scene;
class Camera;

class UnifiedApplication {
public:
    UnifiedApplication();
    virtual ~UnifiedApplication();
    
    // Core lifecycle
    bool Initialize(const platform::WindowConfig& config = {});
    void Run();
    void Shutdown();
    
    // Main loop callback (for web platform)
    void MainLoop();
    
    // Event handling
    virtual void OnEvent(const platform::Event& event);
    virtual void OnUpdate(float deltaTime);
    virtual void OnRender();
    
    // Application state
    bool ShouldClose() const;
    void Close();
    
    // Access to core systems
    UnifiedRenderer* GetRenderer() { return m_Renderer.get(); }
    platform::IPlatform* GetPlatform() { return m_Platform.get(); }
    Scene* GetScene() { return m_Scene.get(); }
    
    // CODA-compatible interface
    void SetBackgroundBrightness(float brightness);
    void AddTestGeometry();
    void Resize(int width, int height);
    
    // Singleton access (CODA-style)
    static UnifiedApplication& Get() {
        static UnifiedApplication instance;
        return instance;
    }

protected:
    // Core systems
    std::unique_ptr<platform::IPlatform> m_Platform;
    std::unique_ptr<UnifiedRenderer> m_Renderer;
    std::unique_ptr<Scene> m_Scene;
    std::unique_ptr<Camera> m_Camera;
    
    // Application state
    bool m_IsInitialized = false;
    bool m_ShouldClose = false;
    float m_LastFrameTime = 0.0f;
    
    // Event handling
    void HandleEvent(const platform::Event& event);
    
    // Platform-specific initialization
    bool InitializePlatform(const platform::WindowConfig& config);
    bool InitializeRenderer();
    bool InitializeScene();
    
    // Main loop implementation
    void UpdateFrame();
    void RenderFrame();
    
    // Web-specific main loop function
    static void WebMainLoop();
    static UnifiedApplication* s_Instance;
};

// Forward declarations for Scene and Camera
class Scene;
class Camera;

} // namespace alice2
