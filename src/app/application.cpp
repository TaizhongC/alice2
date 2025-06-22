#include "application.h"

#include <iostream>
#include <glfw/glfw3.h>

#include "scene.h"
#include "renderer.h"
#include "input.h"

namespace alice2
{
    Application::Application() : m_Window(nullptr)
    {
    }

    Application::~Application()
    {
        if (m_Renderer)
        {
            m_Renderer->Terminate();
        }
        if (m_Window)
        {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    bool Application::Initialize()
    {
        if (!glfwInit())
        {
            return false;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(640, 480, "alice2", nullptr, nullptr);
        if (!m_Window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            return false;
        }

        Input::Initialize(m_Window);
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        m_Renderer = std::make_unique<Renderer>();
        if (!m_Renderer->Initialize(m_Window))
        {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }
        
        m_Scene = std::make_unique<Scene>();

        return true;
    }

    void Application::Run()
    {
        float lastFrameTime = 0.0f;
        while (!glfwWindowShouldClose(m_Window))
        {
            float currentFrameTime = (float)glfwGetTime();
            float deltaTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;

            m_Scene->OnUpdate(deltaTime);
            m_Renderer->Render();
            glfwPollEvents();
        }
    }
} // namespace alice2