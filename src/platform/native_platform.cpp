#include "native_platform.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

namespace alice2 {
namespace platform {

NativePlatform::NativePlatform() : m_Window(nullptr) {
}

NativePlatform::~NativePlatform() {
    Shutdown();
}

bool NativePlatform::Initialize(const WindowConfig& config) {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW for WebGPU
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    m_Window = glfwCreateWindow(config.width, config.height, config.title.c_str(), 
                               config.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    
    if (!m_Window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set up callbacks
    glfwSetWindowUserPointer(m_Window, this);
    
    glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::WindowClose;
            platform->m_EventCallback(event);
        }
    });

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::WindowResize;
            event.data.resize.width = width;
            event.data.resize.height = height;
            platform->m_EventCallback(event);
        }
    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = (action == GLFW_PRESS) ? EventType::KeyPress : EventType::KeyRelease;
            event.data.keyboard.key = key;
            event.data.keyboard.scancode = scancode;
            event.data.keyboard.mods = mods;
            platform->m_EventCallback(event);
        }
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::MouseMove;
            event.data.mouse.x = xpos;
            event.data.mouse.y = ypos;
            platform->m_EventCallback(event);
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = (action == GLFW_PRESS) ? EventType::MousePress : EventType::MouseRelease;
            event.data.mouseButton.button = button;
            event.data.mouseButton.mods = mods;
            platform->m_EventCallback(event);
        }
    });

    glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* platform = static_cast<NativePlatform*>(glfwGetWindowUserPointer(window));
        if (platform && platform->m_EventCallback) {
            Event event;
            event.type = EventType::MouseScroll;
            event.data.scroll.xOffset = xoffset;
            event.data.scroll.yOffset = yoffset;
            platform->m_EventCallback(event);
        }
    });

    std::cout << "Native platform initialized successfully" << std::endl;
    return true;
}

void NativePlatform::Shutdown() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

bool NativePlatform::ShouldClose() {
    return m_Window ? glfwWindowShouldClose(m_Window) : true;
}

void NativePlatform::PollEvents() {
    glfwPollEvents();
}

void NativePlatform::SwapBuffers() {
    // WebGPU handles presentation, so this is a no-op for native
}

std::pair<int, int> NativePlatform::GetFramebufferSize() {
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    return {width, height};
}

std::pair<int, int> NativePlatform::GetWindowSize() {
    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    return {width, height};
}

void NativePlatform::SetWindowSize(int width, int height) {
    glfwSetWindowSize(m_Window, width, height);
}

void* NativePlatform::GetNativeWindow() {
    return m_Window;
}

void NativePlatform::SetEventCallback(EventCallback callback) {
    m_EventCallback = callback;
}

void* NativePlatform::CreateWebGPUSurface(void* instance) {
    return glfwGetWGPUSurface(static_cast<WGPUInstance>(instance), m_Window);
}

bool NativePlatform::IsWeb() const {
    return false;
}

bool NativePlatform::IsNative() const {
    return true;
}

double NativePlatform::GetTime() {
    return glfwGetTime();
}

bool NativePlatform::IsKeyPressed(int key) {
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

std::pair<double, double> NativePlatform::GetMousePosition() {
    double x, y;
    glfwGetCursorPos(m_Window, &x, &y);
    return {x, y};
}

bool NativePlatform::IsMouseButtonPressed(int button) {
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

} // namespace platform
} // namespace alice2
