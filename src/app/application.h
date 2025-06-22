#pragma once

#include <memory>

struct GLFWwindow;

namespace alice2
{
    class Renderer;
    class Scene;

    class Application
    {
    public:
        Application();
        ~Application();

        bool Initialize();
        void Run();

    private:
        GLFWwindow* m_Window;
        std::unique_ptr<Renderer> m_Renderer;
        std::unique_ptr<Scene> m_Scene;
    };
}