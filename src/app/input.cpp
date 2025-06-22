#include "input.h"
#include <glfw/glfw3.h>

namespace alice2
{
    GLFWwindow* Input::s_Window = nullptr;

    void Input::Initialize(GLFWwindow* window)
    {
        s_Window = window;
    }

    bool Input::IsKeyPressed(int key)
    {
        return glfwGetKey(s_Window, key) == GLFW_PRESS;
    }

    void Input::GetMousePosition(double& x, double& y)
    {
        glfwGetCursorPos(s_Window, &x, &y);
    }
} 