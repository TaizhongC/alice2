#pragma once
#include "core/base/Types.h"

namespace alice2
{
    class Camera
    {
    public:
        Camera();
        ~Camera() = default;

        void OnUpdate(float deltaTime);

        const Vec3f& GetPosition() const { return m_Position; }
        const Vec3f& GetFront() const { return m_Front; }
        const Vec3f& GetUp() const { return m_Up; }
        
    private:
        Vec3f m_Position{ 0.0f, 0.0f, 3.0f };
        Vec3f m_Front{ 0.0f, 0.0f, -1.0f };
        Vec3f m_Up{ 0.0f, 1.0f, 0.0f };
        Vec3f m_Right;
        Vec3f m_WorldUp{ 0.0f, 1.0f, 0.0f };

        float m_Yaw = -90.0f;
        float m_Pitch = 0.0f;
        float m_Speed = 2.5f;
        float m_Sensitivity = 0.1f;

        void UpdateCameraVectors();
    };
}