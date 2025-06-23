#pragma once

#include <memory>
#include <vector>
#include "../core/base/Types.h"

namespace alice2 {

// Forward declarations
class UnifiedRenderer;
class Camera;

class Scene {
public:
    Scene();
    ~Scene();

    bool Initialize(UnifiedRenderer* renderer, int width, int height);
    void Update(float deltaTime);
    void Render(UnifiedRenderer* renderer);
    void Cleanup();

    // Object management (simplified)
    void Clear();
    void AddTestGeometry();

    // Camera access
    Camera* GetCamera() { return m_Camera.get(); }

    // Background settings
    void SetBackgroundColor(float brightness);

private:
    std::unique_ptr<Camera> m_Camera;
    bool m_IsInitialized = false;

    // Test geometry data
    std::vector<Vec3f> m_TestPoints;
    std::vector<std::pair<Vec3f, Vec3f>> m_TestLines;

    void CreateTestData();
};

} // namespace alice2