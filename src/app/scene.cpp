#include "scene.h"

namespace alice2
{
    Scene::Scene()
    {
        m_Camera = std::make_unique<Camera>();
    }

    void Scene::OnUpdate(float deltaTime)
    {
        m_Camera->OnUpdate(deltaTime);
    }
} 