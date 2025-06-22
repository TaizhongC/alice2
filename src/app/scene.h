#pragma once

#include <memory>
#include "camera.h"

namespace alice2
{
    class Scene
    {
    public:
        Scene();
        ~Scene() = default;

        void OnUpdate(float deltaTime);

    private:
        std::unique_ptr<Camera> m_Camera;
    };
}