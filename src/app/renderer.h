#pragma once
#include <webgpu/webgpu.hpp>

struct GLFWwindow;

namespace alice2
{
    class Renderer
    {
    public:
        bool Initialize(GLFWwindow* window);
        void Terminate();
        void Render();
        void Test();

    private:
        void InitializePipeline();

        // We put here all the variables that are shared between init and main loop
        int m_Width = 0;
        int m_Height = 0;
        wgpu::Instance m_Instance = nullptr;
        wgpu::Surface m_Surface = nullptr;
        wgpu::Device m_Device = nullptr;
        wgpu::Queue m_Queue = nullptr;
        wgpu::RenderPipeline m_Pipeline = nullptr;
        wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;
    };
} // namespace alice2