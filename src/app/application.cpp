#include "application.h"
#include "renderer.h"
#include <iostream>

namespace alice2
{
    bool Application::Initialize(int width, int height)
    {
        m_Width = width;
        m_Height = height;
        
        // Move the whole initialization here
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        m_Window = glfwCreateWindow(m_Width, m_Height, "alice2", nullptr, nullptr);
        if (!m_Window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            return false;
        }

        m_Renderer = new Renderer();
        if (!m_Renderer->Initialize(m_Window)) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        return true;
    }

        void Application::Terminate()
        {
            m_Renderer->Terminate();
            delete m_Renderer;

            // Move all the release/destroy/terminate calls here
            glfwDestroyWindow(m_Window);
            glfwTerminate();
        }

        void Application::MainLoop()
        {
            glfwPollEvents();
        }

        bool Application::IsRunning()
        {
            return !glfwWindowShouldClose(m_Window);
        }
    } // namespace alice2