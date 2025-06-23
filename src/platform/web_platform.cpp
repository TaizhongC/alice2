#include "web_platform.h"
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLFW/glfw3.h>
#endif

namespace alice2 {
namespace platform {

WebPlatform::WebPlatform() : m_Window(nullptr) {
}

WebPlatform::~WebPlatform() {
    Shutdown();
}

bool WebPlatform::Initialize(const WindowConfig& config) {
#ifdef __EMSCRIPTEN__
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW for web" << std::endl;
        return false;
    }

    // Configure GLFW for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    m_Window = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window for web" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set up callbacks similar to native platform
    glfwSetWindowUserPointer(m_Window, this);
    
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        auto* platform = static_cast<WebPlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::WindowClose;
            platform->m_EventCallback(event);
        }
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        auto* platform = static_cast<WebPlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::WindowResize;
            event.data.resize.width = width;
            event.data.resize.height = height;
            platform->m_EventCallback(event);
        }
    });

    std::cout << "Web platform initialized successfully" << std::endl;
    return true;
#else
    std::cerr << "WebPlatform can only be used with Emscripten" << std::endl;
    return false;
#endif
}

void WebPlatform::Shutdown() {
#ifdef __EMSCRIPTEN__
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
#endif
}

bool WebPlatform::ShouldClose() {
#ifdef __EMSCRIPTEN__
    return m_Window ? glfwWindowShouldClose(m_Window) : true;
#else
    return true;
#endif
}

void WebPlatform::PollEvents() {
#ifdef __EMSCRIPTEN__
    glfwPollEvents();
#endif
}

void WebPlatform::SwapBuffers() {
    // WebGPU handles presentation
}

std::pair<int, int> WebPlatform::GetFramebufferSize() {
#ifdef __EMSCRIPTEN__
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    return {width, height};
#else
    return {800, 600};
#endif
}

std::pair<int, int> WebPlatform::GetWindowSize() {
#ifdef __EMSCRIPTEN__
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    return {width, height};
#else
    return {800, 600};
#endif
}

void WebPlatform::SetWindowSize(int width, int height) {
#ifdef __EMSCRIPTEN__
    glfwSetWindowSize(m_Window, width, height);
#endif
}

void* WebPlatform::GetNativeWindow() {
    return m_Window;
}

void WebPlatform::SetEventCallback(EventCallback callback) {
    m_EventCallback = callback;
}

void* WebPlatform::CreateWebGPUSurface(void* instance) {
#ifdef __EMSCRIPTEN__
    // For Emscripten, WebGPU surface creation is handled differently
    // This will need to be implemented based on the specific WebGPU binding used
    return nullptr;
#else
    return nullptr;
#endif
}

bool WebPlatform::IsWeb() const {
    return true;
}

bool WebPlatform::IsNative() const {
    return false;
}

double WebPlatform::GetTime() {
#ifdef __EMSCRIPTEN__
    return glfwGetTime();
#else
    return 0.0;
#endif
}

bool WebPlatform::IsKeyPressed(int key) {
#ifdef __EMSCRIPTEN__
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
#else
    return false;
#endif
}

std::pair<double, double> WebPlatform::GetMousePosition() {
#ifdef __EMSCRIPTEN__
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    return {x, y};
#else
    return {0.0, 0.0};
#endif
}

bool WebPlatform::IsMouseButtonPressed(int button) {
#ifdef __EMSCRIPTEN__
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
#else
    return false;
#endif
}

} // namespace platform
} // namespace alice2
