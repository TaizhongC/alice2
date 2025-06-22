#include "renderer.h"
#include <iostream>
#include <glfw/glfw3.h>
#include "wgpu_adapter.cpp"
#include "wgpu_device.cpp"

#if defined(WEBGPU_BACKEND_WGPU)
#include <glfw/glfw3native.h>
#endif

namespace alice2
{
    bool Renderer::Initialize(GLFWwindow* /*window*/)
    {
        WGPUInstanceDescriptor desc = {};
        desc.nextInChain = nullptr;
        m_Instance = wgpuCreateInstance(&desc);
        if (!m_Instance)
        {
            std::cerr << "Could not initialize WebGPU!" << std::endl;
            return false;
        }

        std::cout << "WGPU instance: " << m_Instance << std::endl;
        
        WGPURequestAdapterOptions adapterOpts = {};
        adapterOpts.nextInChain = nullptr;
        WGPUAdapter adapter = requestAdapterSync(m_Instance, &adapterOpts);

		std::cout << "Got adapter: " << adapter << std::endl;
        inspectAdapterProperties(adapter);

        WGPUDeviceDescriptor deviceDesc = {};
        deviceDesc.nextInChain = nullptr;
        deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const *message, void * /* pUserData */)
        {
            std::cout << "Device lost: reason " << reason;
            if (message)
                std::cout << " (" << message << ")";
            std::cout << std::endl;
        };

        m_Device = requestDeviceSync(adapter, &deviceDesc);
        std::cout << "Got device: " << m_Device << std::endl;
        wgpuAdapterRelease(adapter);

        auto onDeviceError = [](WGPUErrorType type, char const *message, void * /* pUserData */)
        {
            std::cout << "Uncaptured device error: type " << type;
            if (message)
                std::cout << " (" << message << ")";
            std::cout << std::endl;
        };
        wgpuDeviceSetUncapturedErrorCallback(m_Device, onDeviceError, nullptr /* pUserData */);

        m_Queue = wgpuDeviceGetQueue(m_Device);

        return true;
    }

    void Renderer::Terminate()
    {
        wgpuQueueRelease(m_Queue);
        wgpuDeviceRelease(m_Device);
        wgpuInstanceRelease(m_Instance);
    }
} // namespace alice2
