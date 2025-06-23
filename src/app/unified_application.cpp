#include "unified_application.h"
#include "scene.h"
#include "camera.h"
#include "../platform/platform_interface.h"
#include "../renderer/unified_renderer.h"
#include <iostream>
#include <chrono>

namespace alice2 {

// Static instance for web main loop
UnifiedApplication* UnifiedApplication::s_Instance = nullptr;

UnifiedApplication::UnifiedApplication() {
    s_Instance = this;
}

UnifiedApplication::~UnifiedApplication() {
    Shutdown();
    s_Instance = nullptr;
}

bool UnifiedApplication::Initialize(const platform::WindowConfig& config) {
    if (m_IsInitialized) {
        std::cout << "Application already initialized" << std::endl;
        return true;
    }
    
    std::cout << "Initializing Alice 2 Unified Application..." << std::endl;
    
    // Initialize platform
    if (!InitializePlatform(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return false;
    }
    
    // Initialize renderer
    if (!InitializeRenderer()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // Initialize scene
    if (!InitializeScene()) {
        std::cerr << "Failed to initialize scene" << std::endl;
        return false;
    }
    
    m_IsInitialized = true;
    std::cout << "Alice 2 Unified Application initialized successfully" << std::endl;
    return true;
}

void UnifiedApplication::Run() {
    if (!m_IsInitialized) {
        std::cerr << "Application not initialized" << std::endl;
        return;
    }
    
    std::cout << "Starting main loop..." << std::endl;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!ShouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        UpdateFrame();
        RenderFrame();
        
        m_Platform->PollEvents();
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void UnifiedApplication::Shutdown() {
    if (!m_IsInitialized) {
        return;
    }
    
    std::cout << "Shutting down Alice 2 Unified Application..." << std::endl;
    
    if (m_Scene) {
        m_Scene->Cleanup();
        m_Scene.reset();
    }
    
    if (m_Renderer) {
        m_Renderer->Shutdown();
        m_Renderer.reset();
    }
    
    if (m_Platform) {
        m_Platform->Shutdown();
        m_Platform.reset();
    }
    
    m_IsInitialized = false;
    std::cout << "Alice 2 Unified Application shutdown complete" << std::endl;
}

void UnifiedApplication::MainLoop() {
    if (!m_IsInitialized) {
        return;
    }
    
    auto currentTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = currentTime;
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    UpdateFrame();
    RenderFrame();
}

void UnifiedApplication::OnEvent(const platform::Event& event) {
    HandleEvent(event);
}

void UnifiedApplication::OnUpdate(float deltaTime) {
    if (m_Scene) {
        m_Scene->Update(deltaTime);

        // Process camera input
        if (m_Scene->GetCamera()) {
            m_Scene->GetCamera()->ProcessInput(m_Platform.get(), deltaTime);
        }
    }
}

void UnifiedApplication::OnRender() {
    if (m_Renderer && m_Scene) {
        m_Renderer->BeginFrame();
        m_Scene->Render(m_Renderer.get());
        m_Renderer->EndFrame();
    }
}

bool UnifiedApplication::ShouldClose() const {
    return m_ShouldClose || (m_Platform && m_Platform->ShouldClose());
}

void UnifiedApplication::Close() {
    m_ShouldClose = true;
}

void UnifiedApplication::SetBackgroundBrightness(float brightness) {
    if (m_Scene) {
        m_Scene->SetBackgroundColor(brightness);
    }
}

void UnifiedApplication::AddTestGeometry() {
    if (m_Scene) {
        m_Scene->AddTestGeometry();
    }
}

void UnifiedApplication::Resize(int width, int height) {
    if (m_Renderer) {
        m_Renderer->SetViewport(width, height);
    }
    
    if (m_Scene && m_Scene->GetCamera()) {
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        m_Scene->GetCamera()->SetAspect(aspect);
    }
}

void UnifiedApplication::HandleEvent(const platform::Event& event) {
    switch (event.type) {
        case platform::EventType::WindowClose:
            Close();
            break;
            
        case platform::EventType::WindowResize:
            Resize(event.data.resize.width, event.data.resize.height);
            break;
            
        case platform::EventType::KeyPress:
            // Handle key presses
            break;
            
        case platform::EventType::MouseMove:
            // Handle mouse movement
            break;
            
        default:
            break;
    }
}

bool UnifiedApplication::InitializePlatform(const platform::WindowConfig& config) {
    m_Platform = platform::CreatePlatform();
    if (!m_Platform) {
        std::cerr << "Failed to create platform" << std::endl;
        return false;
    }
    
    // Set event callback
    m_Platform->SetEventCallback([this](const platform::Event& event) {
        OnEvent(event);
    });
    
    if (!m_Platform->Initialize(config)) {
        std::cerr << "Failed to initialize platform" << std::endl;
        return false;
    }
    
    std::cout << "Platform initialized successfully" << std::endl;
    return true;
}

bool UnifiedApplication::InitializeRenderer() {
    m_Renderer = std::make_unique<UnifiedRenderer>();
    if (!m_Renderer->Initialize(m_Platform.get())) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    // Set initial viewport
    auto [width, height] = m_Platform->GetFramebufferSize();
    m_Renderer->SetViewport(width, height);
    
    std::cout << "Renderer initialized successfully" << std::endl;
    return true;
}

bool UnifiedApplication::InitializeScene() {
    m_Scene = std::make_unique<Scene>();
    auto [width, height] = m_Platform->GetFramebufferSize();
    
    if (!m_Scene->Initialize(m_Renderer.get(), width, height)) {
        std::cerr << "Failed to initialize scene" << std::endl;
        return false;
    }
    
    std::cout << "Scene initialized successfully" << std::endl;
    return true;
}

void UnifiedApplication::UpdateFrame() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = currentTime;
    float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    OnUpdate(deltaTime);
}

void UnifiedApplication::RenderFrame() {
    OnRender();
}

void UnifiedApplication::WebMainLoop() {
    if (s_Instance) {
        s_Instance->MainLoop();
    }
}

} // namespace alice2
