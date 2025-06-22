#include "renderer.h"
#include <iostream>
#include <vector>
#include <glfw/glfw3.h>
#include <webgpu/webgpu.h>

#if defined(WEBGPU_BACKEND_WGPU)
#include <glfw3webgpu.h>
#include <glfw/glfw3native.h>
#endif

namespace alice2
{
    using namespace wgpu;

    bool Renderer::Initialize(GLFWwindow* window)
    {
        // Create instance
        m_Instance = createInstance(InstanceDescriptor{});
        if (!m_Instance)
        {
            std::cerr << "Could not initialize WebGPU!" << std::endl;
            return false;
        }
        std::cout << "WGPU instance: " << m_Instance << std::endl;

        // Create surface
        m_Surface = glfwGetWGPUSurface(m_Instance, window);

        // Request adapter
        RequestAdapterOptions adapterOpts{};
        adapterOpts.compatibleSurface = m_Surface;

        Adapter adapter = nullptr;
        auto adapterCallback = [&adapter](RequestAdapterStatus status, Adapter ad, char const* message) {
            if (status == RequestAdapterStatus::Success) {
                adapter = ad;
            } else {
                std::cout << "Could not get WebGPU adapter: " << message << std::endl;
            }
        };
        
        m_Instance.requestAdapter(adapterOpts, adapterCallback);
        
        if (!adapter) {
            std::cerr << "Failed to get adapter" << std::endl;
            return false;
        }
        std::cout << "Got adapter: " << adapter << std::endl;

        // Request device
        DeviceDescriptor deviceDesc{};
        deviceDesc.label = "My Device";

        auto deviceCallback = [this](RequestDeviceStatus status, Device dev, char const* message) {
            if (status == RequestDeviceStatus::Success) {
                m_Device = std::move(dev);
            } else {
                std::cout << "Could not get WebGPU device: " << message << std::endl;
            }
        };

        adapter.requestDevice(deviceDesc, deviceCallback);
        
        if (!m_Device) {
            std::cerr << "Failed to get device" << std::endl;
            return false;
        }
        std::cout << "Got device: " << m_Device << std::endl;

        // Get preferred format before releasing adapter
        m_SurfaceFormat = m_Surface.getPreferredFormat(adapter);
        adapter.release();

        // Get queue
        m_Queue = m_Device.getQueue();

        // Configure surface
        SurfaceConfiguration config{};
        config.device = m_Device;
        config.usage = TextureUsage::RenderAttachment;
        config.format = m_SurfaceFormat;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        config.width = width;
        m_Width = width;
        config.height = height;
        m_Height = height;

        config.presentMode = PresentMode::Fifo;

        m_Surface.configure(config);

        InitializePipeline();

        return true;
    }

    void Renderer::InitializePipeline() {
        const char* shaderSource = R"(
            @vertex
            fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
                var p = vec2f(0.0, 0.0);
                if (in_vertex_index == 0u) {
                    p = vec2f(-0.5, -0.5);
                } else if (in_vertex_index == 1u) {
                    p = vec2f(0.5, -0.5);
                } else {
                    p = vec2f(0.0, 0.5);
                }
                return vec4f(p, 0.0, 1.0);
            }

            @fragment
            fn fs_main() -> @location(0) vec4f {
                return vec4f(0.0, 0.4, 1.0, 1.0);
            }
        )";

        ShaderModuleWGSLDescriptor shaderCodeDesc{};
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
        shaderCodeDesc.code = shaderSource;

        ShaderModuleDescriptor shaderDesc{};
        shaderDesc.nextInChain = &shaderCodeDesc.chain;
        ShaderModule shaderModule = m_Device.createShaderModule(shaderDesc);

        RenderPipelineDescriptor pipelineDesc{};
        pipelineDesc.layout = nullptr;

        // Vertex state
        pipelineDesc.vertex.module = shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.bufferCount = 0;
        pipelineDesc.vertex.buffers = nullptr;

        // Primitive state
        pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
        pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
        pipelineDesc.primitive.frontFace = FrontFace::CCW;
        pipelineDesc.primitive.cullMode = CullMode::None;

        // Fragment state
        FragmentState fragmentState{};
        pipelineDesc.fragment = &fragmentState;
        fragmentState.module = shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.targetCount = 1;

        ColorTargetState colorTarget{};
        colorTarget.format = m_SurfaceFormat;
        colorTarget.blend = nullptr;
        colorTarget.writeMask = ColorWriteMask::All;
        fragmentState.targets = &colorTarget;

        // Depth/stencil state
        pipelineDesc.depthStencil = nullptr;

        // Multisample state
        pipelineDesc.multisample.count = 1;
        pipelineDesc.multisample.mask = ~0u;
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        m_Pipeline = m_Device.createRenderPipeline(pipelineDesc);
        shaderModule.release();
    }

    void Renderer::Terminate()
    {
        if (m_Pipeline) {
            m_Pipeline.release();
        }
        if (m_Surface) {
            m_Surface.unconfigure();
            m_Surface.release();
        }
        if (m_Queue) {
            m_Queue.release();
        }
        if (m_Device) {
            m_Device.release();
        }
        if (m_Instance) {
            m_Instance.release();
        }
    }

    void Renderer::Render()
    {
        SurfaceTexture surfaceTexture{};
        m_Surface.getCurrentTexture(&surfaceTexture);

        if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
            return;
        }

        wgpu::Texture texture(surfaceTexture.texture);
        TextureView targetView = texture.createView();

        CommandEncoderDescriptor encoderDesc{};
        CommandEncoder encoder = m_Device.createCommandEncoder(encoderDesc);

        RenderPassDescriptor renderPassDesc{};
        RenderPassColorAttachment colorAttachment{};
        colorAttachment.view = targetView;
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearValue = { 0.9, 0.1, 0.2, 1.0 };
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &colorAttachment;

        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.setPipeline(m_Pipeline);
        renderPass.draw(3, 1, 0, 0);
        renderPass.end();

        CommandBufferDescriptor cmdBufferDescriptor{};
        CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        m_Queue.submit(command);

        m_Surface.present();

        // Release resources
        command.release();
        encoder.release();
        renderPass.release();
        targetView.release();
        texture.release();
    }
}