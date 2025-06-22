#include "renderer.h"
#include <iostream>
#include <vector>
#include <glfw/glfw3.h>

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
		// deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* userData)
		// {
		// 	std::cout << "Device lost: reason " << reason;
		// 	if (message)
		// 		std::cout << " (" << message << ")";
		// 	std::cout << std::endl;
		// };

		adapter.requestDevice(deviceDesc, [this](RequestDeviceStatus status, Device dev, char const * message) {
			if (status == RequestDeviceStatus::Success) {
				m_Device = std::move(dev);
			} else {
				std::cout << "Could not get WebGPU device: " << message << std::endl;
			}
		});
		std::cout << "Got device: " << m_Device << std::endl;
        adapter.release();

		// m_Device.setUncapturedErrorCallback([](ErrorType type, char const* message, void* userData)
		// {
		// 	std::cout << "Uncaptured device error: type " << type;
		// 	if (message)
		// 		std::cout << " (" << message << ")";
		// 	std::cout << std::endl;
		// });

		m_Queue = m_Device.getQueue();

		SurfaceConfiguration config{};
		config.device = m_Device;
		config.usage = TextureUsage::RenderAttachment;
		TextureFormat surfaceFormat = m_Surface.getPreferredFormat(adapter);
		config.format = surfaceFormat;

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		config.width = width;
		m_Width = width;
		config.height = height;
		m_Height = height;

		config.presentMode = PresentMode::Fifo;

		m_Surface.configure(config);

		return true;
	}

	void Renderer::Terminate()
	{
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
