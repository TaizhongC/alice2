#pragma once
#include "../core/base/Types.h"

namespace alice2 {

// Forward declarations
namespace platform { class IPlatform; }

class Camera {
public:
    Camera();
    ~Camera() = default;

    void Update(float deltaTime);
    void SetAspect(float aspect);
    void SetPosition(const Vec3f& position);
    void SetTarget(const Vec3f& target);

    // Matrix access
    void GetViewMatrix(float* matrix) const;
    void GetProjectionMatrix(float* matrix) const;

    // Camera controls
    void ProcessInput(platform::IPlatform* platform, float deltaTime);

    // Orbital camera controls
    void SetDistance(float distance);
    void SetAngles(float yaw, float pitch);
    void Orbit(float deltaYaw, float deltaPitch);
    void Zoom(float deltaDistance);

    // Getters
    const Vec3f& GetPosition() const { return m_Position; }
    const Vec3f& GetTarget() const { return m_Target; }
    float GetDistance() const { return m_Distance; }

private:
    Vec3f m_Position = Vec3f(0, 0, 5);
    Vec3f m_Target = Vec3f(0, 0, 0);
    Vec3f m_Up = Vec3f(0, 1, 0);

    // Orbital camera parameters
    float m_Distance = 5.0f;
    float m_Yaw = 0.0f;      // Rotation around Y axis
    float m_Pitch = 0.0f;    // Rotation around X axis

    float m_Fov = 45.0f;
    float m_Aspect = 16.0f / 9.0f;
    float m_Near = 0.1f;
    float m_Far = 100.0f;

    // Camera movement
    float m_MoveSpeed = 5.0f;
    float m_RotateSpeed = 2.0f;
    float m_ZoomSpeed = 1.0f;

    // Input state
    bool m_MousePressed = false;
    double m_LastMouseX = 0.0;
    double m_LastMouseY = 0.0;

    void UpdateMatrices();
    void UpdatePositionFromAngles();
    void CreateViewMatrix(float* matrix) const;
    void CreateProjectionMatrix(float* matrix) const;
};

} // namespace alice2