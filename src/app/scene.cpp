#include "scene.h"
#include "camera.h"
#include "../renderer/unified_renderer.h"
#include <iostream>
#include <cmath>

namespace alice2 {

Scene::Scene() {
}

Scene::~Scene() {
    Cleanup();
}

bool Scene::Initialize(UnifiedRenderer* renderer, int width, int height) {
    if (m_IsInitialized) {
        return true;
    }

    // Create camera
    m_Camera = std::make_unique<Camera>();
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_Camera->SetAspect(aspect);

    // Create some test data
    CreateTestData();

    m_IsInitialized = true;
    std::cout << "Scene initialized successfully" << std::endl;
    return true;
}

void Scene::Update(float deltaTime) {
    if (m_Camera) {
        m_Camera->Update(deltaTime);

        // For now, we can add some simple animation to test the rendering
        static float time = 0.0f;
        time += deltaTime;

        // Animate some test points
        if (!m_TestPoints.empty() && m_TestPoints.size() > 4) {
            // Animate the extra points in a circle
            for (size_t i = 4; i < m_TestPoints.size(); ++i) {
                float angle = time + (i - 4) * 0.5f;
                float radius = 2.0f;
                m_TestPoints[i].x = radius * std::cos(angle);
                m_TestPoints[i].z = radius * std::sin(angle);
                m_TestPoints[i].y = std::sin(time * 2.0f + i) * 0.5f;
            }
        }
    }
}

void Scene::Render(UnifiedRenderer* renderer) {
    if (!renderer) {
        return;
    }

    // DEBUGGING: Test with identity matrices to render directly in NDC space
    float identityMatrix[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    renderer->SetViewMatrix(identityMatrix);
    renderer->SetProjectionMatrix(identityMatrix);

    // Simple NDC test - render basic geometry directly in normalized device coordinates

    // Render a simple triangle directly in NDC space (-1 to 1 range)
    renderer->BeginTriangles();
    Vec3f p0(-0.8f, -0.8f, 0.0f);  // Bottom left
    Vec3f p1(0.8f, -0.8f, 0.0f);   // Bottom right
    Vec3f p2(0.0f, 0.8f, 0.0f);    // Top center
    Color triangleColor(1.0f, 0.0f, 0.0f, 1.0f); // Bright red, fully opaque
    renderer->AddTriangle(p0, p1, p2, triangleColor);
    renderer->EndTriangles();

    // Render a simple line directly in NDC space
    renderer->BeginLines();
    Vec3f lineStart(-0.9f, 0.0f, 0.0f);
    Vec3f lineEnd(0.9f, 0.0f, 0.0f);
    Color lineColor(0.0f, 1.0f, 0.0f, 1.0f); // Bright green
    renderer->AddLine(lineStart, lineEnd, lineColor);
    renderer->EndLines();
}

void Scene::Cleanup() {
    m_Camera.reset();
    m_TestPoints.clear();
    m_TestLines.clear();
    m_IsInitialized = false;
}

void Scene::Clear() {
    m_TestPoints.clear();
    m_TestLines.clear();
}

void Scene::AddTestGeometry() {
    CreateTestData();
}

void Scene::SetBackgroundColor(float brightness) {
    // This would be handled by the renderer
    std::cout << "Setting background brightness to: " << brightness << std::endl;
}

void Scene::CreateTestData() {
    // Clear existing data
    m_TestPoints.clear();
    m_TestLines.clear();

    // Create coordinate system points
    m_TestPoints.push_back(Vec3f(0.0f, 0.0f, 0.0f));    // Origin
    m_TestPoints.push_back(Vec3f(1.0f, 0.0f, 0.0f));    // X axis
    m_TestPoints.push_back(Vec3f(0.0f, 1.0f, 0.0f));    // Y axis
    m_TestPoints.push_back(Vec3f(0.0f, 0.0f, 1.0f));    // Z axis

    // Add some animated points
    for (int i = 0; i < 8; ++i) {
        m_TestPoints.push_back(Vec3f(0.0f, 0.0f, 0.0f)); // Will be animated in Update()
    }

    // Create coordinate axes lines (with different colors conceptually)
    m_TestLines.push_back({Vec3f(0.0f, 0.0f, 0.0f), Vec3f(2.0f, 0.0f, 0.0f)});  // X axis (red)
    m_TestLines.push_back({Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 2.0f, 0.0f)});  // Y axis (green)
    m_TestLines.push_back({Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 2.0f)});  // Z axis (blue)

    // Create a cube wireframe
    float s = 0.5f; // Half size
    Vec3f vertices[8] = {
        Vec3f(-s, -s, -s), Vec3f( s, -s, -s), Vec3f( s,  s, -s), Vec3f(-s,  s, -s),
        Vec3f(-s, -s,  s), Vec3f( s, -s,  s), Vec3f( s,  s,  s), Vec3f(-s,  s,  s)
    };

    // Bottom face
    m_TestLines.push_back({vertices[0], vertices[1]});
    m_TestLines.push_back({vertices[1], vertices[2]});
    m_TestLines.push_back({vertices[2], vertices[3]});
    m_TestLines.push_back({vertices[3], vertices[0]});

    // Top face
    m_TestLines.push_back({vertices[4], vertices[5]});
    m_TestLines.push_back({vertices[5], vertices[6]});
    m_TestLines.push_back({vertices[6], vertices[7]});
    m_TestLines.push_back({vertices[7], vertices[4]});

    // Vertical edges
    m_TestLines.push_back({vertices[0], vertices[4]});
    m_TestLines.push_back({vertices[1], vertices[5]});
    m_TestLines.push_back({vertices[2], vertices[6]});
    m_TestLines.push_back({vertices[3], vertices[7]});

    // Create a grid on the ground plane
    float gridSize = 5.0f;
    int gridLines = 10;
    float step = gridSize * 2.0f / gridLines;

    for (int i = 0; i <= gridLines; ++i) {
        float pos = -gridSize + i * step;
        // Lines parallel to X axis
        m_TestLines.push_back({Vec3f(-gridSize, -1.0f, pos), Vec3f(gridSize, -1.0f, pos)});
        // Lines parallel to Z axis
        m_TestLines.push_back({Vec3f(pos, -1.0f, -gridSize), Vec3f(pos, -1.0f, gridSize)});
    }

    // Create a spiral
    int spiralPoints = 50;
    for (int i = 0; i < spiralPoints - 1; ++i) {
        float t1 = static_cast<float>(i) / spiralPoints * 4.0f * 3.14159f;
        float t2 = static_cast<float>(i + 1) / spiralPoints * 4.0f * 3.14159f;

        Vec3f p1(std::cos(t1) * (1.0f + t1 * 0.1f), t1 * 0.2f, std::sin(t1) * (1.0f + t1 * 0.1f));
        Vec3f p2(std::cos(t2) * (1.0f + t2 * 0.1f), t2 * 0.2f, std::sin(t2) * (1.0f + t2 * 0.1f));

        m_TestLines.push_back({p1, p2});
    }

    std::cout << "Created enhanced test geometry: " << m_TestPoints.size() << " points, " << m_TestLines.size() << " lines" << std::endl;
}

} // namespace alice2