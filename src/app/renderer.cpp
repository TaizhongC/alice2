#include "renderer.h"
#include <iostream>
#include <glfw/glfw3.h>
#include <glfw3webgpu.h>
#include "wgpu_adapter.cpp"
#include "wgpu_device.cpp"

#if defined(WEBGPU_BACKEND_WGPU)
#include <glfw/glfw3native.h>
#endif

namespace alice2
{
    bool Renderer::Initialize(GLFWwindow* window)
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
        
#if defined(WEBGPU_BACKEND_WGPU)
		m_Surface = glfwGetWGPUSurface(m_Instance, window);
#endif
        
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

        WGPUSurfaceConfiguration config = {};
		config.nextInChain = nullptr;
		config.device = m_Device;
		config.usage = WGPUTextureUsage_RenderAttachment;
        WGPUTextureFormat surfaceFormat = wgpuSurfaceGetPreferredFormat(m_Surface, adapter);
        config.format = surfaceFormat;
        
        int width, height;
		glfwGetFramebufferSize(window, &width, &height);
        config.width = width;
        config.height = height;
        
        config.presentMode = WGPUPresentMode_Fifo;

        wgpuSurfaceConfigure(m_Surface, &config);

        return true;
    }

    void Renderer::Terminate()
    {
        wgpuSurfaceRelease(m_Surface);
        wgpuQueueRelease(m_Queue);
        wgpuDeviceRelease(m_Device);
        wgpuInstanceRelease(m_Instance);
    }

    void Renderer::Render()
    {
        WGPUSurfaceTexture surfaceTexture;
		wgpuSurfaceGetCurrentTexture(m_Surface, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
			return;
		}

        WGPUTextureView targetView = wgpuTextureCreateView(surfaceTexture.texture, nullptr);

        WGPUCommandEncoderDescriptor encoderDesc = {};
		WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Device, &encoderDesc);

        WGPURenderPassDescriptor renderPassDesc = {};
        WGPURenderPassColorAttachment colorAttachment = {};
		colorAttachment.view = targetView;
		colorAttachment.resolveTarget = nullptr;
		colorAttachment.loadOp = WGPULoadOp_Clear;
		colorAttachment.storeOp = WGPUStoreOp_Store;
		colorAttachment.clearValue = { 0.9, 0.1, 0.2, 1.0 };
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;
        renderPassDesc.depthStencilAttachment = nullptr;
        
        WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
		wgpuRenderPassEncoderEnd(renderPass);
		wgpuRenderPassEncoderRelease(renderPass);

        WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
		WGPUCommandBuffer command = wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
		wgpuQueueSubmit(m_Queue, 1, &command);

        wgpuTextureViewRelease(targetView);
        
        wgpuSurfacePresent(m_Surface);
    }
} // namespace alice2
