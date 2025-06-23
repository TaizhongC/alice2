#include "camera.h"
#include "unified_application.h"
#include "../platform/platform_interface.h"
#include <cmath>
#include <iostream>

namespace alice2 {

Camera::Camera() {
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::Update(float deltaTime) {
    // Update matrices if needed
    UpdateMatrices();
}

void Camera::SetAspect(float aspect) {
    m_Aspect = aspect;
    UpdateMatrices();
}

void Camera::SetPosition(const Vec3f& position) {
    m_Position = position;
    // Calculate distance and angles from new position
    Vec3f toTarget = m_Target - m_Position;
    m_Distance = toTarget.Length();
    if (m_Distance > 0.001f) {
        toTarget = toTarget.Normalize();
        m_Pitch = std::asin(-toTarget.y);
        m_Yaw = std::atan2(toTarget.x, toTarget.z);
    }
    UpdateMatrices();
}

void Camera::SetTarget(const Vec3f& target) {
    m_Target = target;
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::SetDistance(float distance) {
    m_Distance = std::max(0.1f, distance);
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::SetAngles(float yaw, float pitch) {
    m_Yaw = yaw;
    m_Pitch = std::max(-1.5f, std::min(1.5f, pitch)); // Clamp pitch to avoid gimbal lock
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::Orbit(float deltaYaw, float deltaPitch) {
    m_Yaw += deltaYaw;
    m_Pitch += deltaPitch;
    m_Pitch = std::max(-1.5f, std::min(1.5f, m_Pitch)); // Clamp pitch
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::Zoom(float deltaDistance) {
    m_Distance += deltaDistance;
    m_Distance = std::max(0.1f, std::min(100.0f, m_Distance)); // Clamp distance
    UpdatePositionFromAngles();
    UpdateMatrices();
}

void Camera::GetViewMatrix(float* matrix) const {
    CreateViewMatrix(matrix);
}

void Camera::GetProjectionMatrix(float* matrix) const {
    CreateProjectionMatrix(matrix);
}

void Camera::ProcessInput(platform::IPlatform* platform, float deltaTime) {
    if (!platform) return;

    // Mouse input for orbital camera
    auto [mouseX, mouseY] = platform->GetMousePosition();
    bool mousePressed = platform->IsMouseButtonPressed(0); // Left mouse button

    if (mousePressed) {
        if (m_MousePressed) {
            // Calculate mouse delta
            double deltaX = mouseX - m_LastMouseX;
            double deltaY = mouseY - m_LastMouseY;

            // Apply rotation
            float rotateSpeed = m_RotateSpeed * deltaTime;
            Orbit(static_cast<float>(-deltaX * rotateSpeed), static_cast<float>(-deltaY * rotateSpeed));
        }
        m_MousePressed = true;
    } else {
        m_MousePressed = false;
    }

    m_LastMouseX = mouseX;
    m_LastMouseY = mouseY;

    // Keyboard input for camera movement
    float moveSpeed = m_MoveSpeed * deltaTime;
    float zoomSpeed = m_ZoomSpeed * deltaTime * 10.0f;

    // WASD for target movement
    Vec3f forward = (m_Target - m_Position).Normalize();
    Vec3f right = forward.Cross(m_Up).Normalize();
    Vec3f up = m_Up;

    if (platform->IsKeyPressed(87)) { // W key - move target forward
        m_Target += forward * moveSpeed;
        UpdatePositionFromAngles();
    }
    if (platform->IsKeyPressed(83)) { // S key - move target backward
        m_Target -= forward * moveSpeed;
        UpdatePositionFromAngles();
    }
    if (platform->IsKeyPressed(65)) { // A key - move target left
        m_Target -= right * moveSpeed;
        UpdatePositionFromAngles();
    }
    if (platform->IsKeyPressed(68)) { // D key - move target right
        m_Target += right * moveSpeed;
        UpdatePositionFromAngles();
    }
    if (platform->IsKeyPressed(81)) { // Q key - move target up
        m_Target += up * moveSpeed;
        UpdatePositionFromAngles();
    }
    if (platform->IsKeyPressed(69)) { // E key - move target down
        m_Target -= up * moveSpeed;
        UpdatePositionFromAngles();
    }

    // Mouse wheel or +/- keys for zoom
    if (platform->IsKeyPressed(187)) { // + key
        Zoom(-zoomSpeed);
    }
    if (platform->IsKeyPressed(189)) { // - key
        Zoom(zoomSpeed);
    }

    UpdateMatrices();
}

void Camera::UpdateMatrices() {
    // This will be called whenever camera parameters change
    // Matrices are calculated on-demand in Get methods
}

void Camera::UpdatePositionFromAngles() {
    // Calculate position from spherical coordinates
    float cosYaw = std::cos(m_Yaw);
    float sinYaw = std::sin(m_Yaw);
    float cosPitch = std::cos(m_Pitch);
    float sinPitch = std::sin(m_Pitch);

    // Position relative to target
    Vec3f offset;
    offset.x = m_Distance * cosPitch * sinYaw;
    offset.y = m_Distance * sinPitch;
    offset.z = m_Distance * cosPitch * cosYaw;

    m_Position = m_Target + offset;
}

void Camera::CreateViewMatrix(float* matrix) const {
    // Create a proper look-at matrix
    Vec3f forward = (m_Target - m_Position).Normalize();
    Vec3f right = forward.Cross(m_Up).Normalize();
    Vec3f up = right.Cross(forward).Normalize();

    // Initialize to zero
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }

    // Set rotation part (transpose of camera orientation)
    matrix[0] = right.x;    matrix[4] = right.y;    matrix[8] = right.z;     matrix[12] = 0.0f;
    matrix[1] = up.x;       matrix[5] = up.y;       matrix[9] = up.z;        matrix[13] = 0.0f;
    matrix[2] = -forward.x; matrix[6] = -forward.y; matrix[10] = -forward.z; matrix[14] = 0.0f;
    matrix[3] = 0.0f;       matrix[7] = 0.0f;       matrix[11] = 0.0f;       matrix[15] = 1.0f;

    // Apply translation
    matrix[12] = -(right.x * m_Position.x + right.y * m_Position.y + right.z * m_Position.z);
    matrix[13] = -(up.x * m_Position.x + up.y * m_Position.y + up.z * m_Position.z);
    matrix[14] = -(-forward.x * m_Position.x + -forward.y * m_Position.y + -forward.z * m_Position.z);
}

void Camera::CreateProjectionMatrix(float* matrix) const {
    // Create perspective projection matrix
    float fovRad = m_Fov * (3.14159265359f / 180.0f);
    float tanHalfFov = std::tan(fovRad * 0.5f);

    // Initialize to zero
    for (int i = 0; i < 16; ++i) {
        matrix[i] = 0.0f;
    }

    matrix[0] = 1.0f / (m_Aspect * tanHalfFov);
    matrix[5] = 1.0f / tanHalfFov;
    matrix[10] = -(m_Far + m_Near) / (m_Far - m_Near);
    matrix[11] = -1.0f;
    matrix[14] = -(2.0f * m_Far * m_Near) / (m_Far - m_Near);
}

} // namespace alice2