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
		m_Instance = createInstance(InstanceDescriptor{});
		if (!m_Instance)
		{
			std::cerr << "Could not initialize WebGPU!" << std::endl;
			return false;
		}
		std::cout << "WGPU instance: " << m_Instance << std::endl;

		m_Surface = glfwGetWGPUSurface(m_Instance, window);

		RequestAdapterOptions adapterOpts{};
		adapterOpts.compatibleSurface = m_Surface;

		Adapter adapter = nullptr;
		m_Instance.requestAdapter(adapterOpts, [&adapter](RequestAdapterStatus status, Adapter ad, char const * message) {
			if (status == RequestAdapterStatus::Success) {
				adapter = ad;
			} else {
				std::cout << "Could not get WebGPU adapter: " << message << std::endl;
			}
		});
		std::cout << "Got adapter: " << adapter << std::endl;

		DeviceDescriptor deviceDesc{};
		deviceDesc.label = "My Device";

		adapter.requestDevice(deviceDesc, [this](RequestDeviceStatus status, Device dev, char const * message) {
			if (status == RequestDeviceStatus::Success) {
				m_Device = std::move(dev);
			} else {
				std::cout << "Could not get WebGPU device: " << message << std::endl;
			}
		});
		std::cout << "Got device: " << m_Device << std::endl;
        adapter.release();

		m_Queue = m_Device.getQueue();

		SurfaceConfiguration config{};
		config.device = m_Device;
		config.usage = TextureUsage::RenderAttachment;
		m_SurfaceFormat = m_Surface.getPreferredFormat(adapter);
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

	void Renderer::Test()
	{
		// Experimentation for the "Playing with buffers" chapter from @https://eliemichel.github.io/LearnWebGPU/basic-3d-rendering/input-geometry/playing-with-buffers.html
		{
			// Create a source buffer
			BufferDescriptor bufferDesc;
			bufferDesc.label = "Source buffer";
			bufferDesc.usage = BufferUsage::CopySrc | BufferUsage::CopyDst;
			bufferDesc.size = 16;
			Buffer buffer1 = m_Device.createBuffer(bufferDesc);

			// Create data to write to buffer1
			std::vector<uint8_t> myData(16, 0);
			for (uint8_t i = 0; i < myData.size(); ++i) {
				myData[i] = i;
			}
			m_Queue.writeBuffer(buffer1, 0, myData.data(), myData.size());

			// Create a destination buffer
			bufferDesc.label = "Destination buffer";
			bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::MapRead;
			bufferDesc.size = 16;
			Buffer buffer2 = m_Device.createBuffer(bufferDesc);

			// Create command encoder to copy
			CommandEncoderDescriptor encoderDesc{};
			encoderDesc.label = "My command encoder";
			CommandEncoder commandEncoder = m_Device.createCommandEncoder(encoderDesc);
			commandEncoder.copyBufferToBuffer(buffer1, 0, buffer2, 0, 16);
			CommandBuffer command = commandEncoder.finish(CommandBufferDescriptor{});
			m_Queue.submit(command);

			// Read back buffer 2
			bool finished = false;
			buffer2.mapAsync(MapMode::Read, 0, 16, [&](WGPUBufferMapAsyncStatus status) {
				if (status == WGPUBufferMapAsyncStatus_Success) {
					const uint8_t* data = (const uint8_t*)buffer2.getConstMappedRange(0, 16);
					std::cout << "Buffer content: [";
					for (int i = 0; i < 16; ++i) {
						std::cout << (i > 0 ? ", " : "") << (int)data[i];
					}
					std::cout << "]" << std::endl;
					buffer2.unmap();
				}
				else {
					std::cout << "Failed to map buffer for reading" << std::endl;
				}
				finished = true;
			});

			// Poll for results
			#ifndef __EMSCRIPTEN__
			while (!finished) {
				wgpuDevicePoll(m_Device, true, nullptr);
			}
			#endif

			// Release resources
			buffer1.release();
			buffer2.release();
			commandEncoder.release();
			command.release();
		}
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
		m_Pipeline.release();
        m_Surface.unconfigure();
		m_Queue.release();
		m_Device.release();
		m_Surface.release();
		m_Instance.release();
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
        targetView.release();
        texture.release();
	}
} // namespace alice2
