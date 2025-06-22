#pragma once

struct GLFWwindow;

namespace alice2
{
    class Input
    {
    public:
        static void Initialize(GLFWwindow* window);
        static bool IsKeyPressed(int key);
        static void GetMousePosition(double& x, double& y);

    private:
        static GLFWwindow* s_Window;
    };
} 