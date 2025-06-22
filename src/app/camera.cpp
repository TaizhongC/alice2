#include "camera.h"
#include "input.h"
#include <glfw/glfw3.h>
#include <cmath>

namespace alice2
{
    Camera::Camera()
        : m_Yaw(-90.0f), m_Pitch(0.0f)
    {
        UpdateCameraVectors();
    }

    void Camera::OnUpdate(float deltaTime)
    {
        // Keyboard input
        float velocity = m_Speed * deltaTime;
        if (Input::IsKeyPressed(GLFW_KEY_W))
            m_Position += m_Front * velocity;
        if (Input::IsKeyPressed(GLFW_KEY_S))
            m_Position -= m_Front * velocity;
        if (Input::IsKeyPressed(GLFW_KEY_A))
            m_Position -= m_Right * velocity;
        if (Input::IsKeyPressed(GLFW_KEY_D))
            m_Position += m_Right * velocity;

        // Mouse input
        static double lastX = 0.0, lastY = 0.0;
        static bool firstMouse = true;
        double xpos, ypos;
        Input::GetMousePosition(xpos, ypos);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = (float)(xpos - lastX);
        float yoffset = (float)(lastY - ypos);
        lastX = xpos;
        lastY = ypos;

        xoffset *= m_Sensitivity;
        yoffset *= m_Sensitivity;

        m_Yaw += xoffset;
        m_Pitch += yoffset;

        if (m_Pitch > 89.0f)
            m_Pitch = 89.0f;
        if (m_Pitch < -89.0f)
            m_Pitch = -89.0f;

        UpdateCameraVectors();
    }

    void Camera::UpdateCameraVectors()
    {
        Vec3f front;
        front.x = cos(m_Yaw * DEG_TO_RAD) * cos(m_Pitch * DEG_TO_RAD);
        front.y = sin(m_Pitch * DEG_TO_RAD);
        front.z = sin(m_Yaw * DEG_TO_RAD) * cos(m_Pitch * DEG_TO_RAD);
        m_Front = front.Normalize();
        
        m_Right = m_Front.Cross(m_WorldUp).Normalize();
        m_Up = m_Right.Cross(m_Front).Normalize();
    }
}