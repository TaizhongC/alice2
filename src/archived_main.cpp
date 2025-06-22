#include <iostream>
#include <webgpu/webgpu.h>
#include "wgpu_adapter.cpp"
#include "wgpu_device.cpp"
#include <vector>
#include <glfw/glfw3.h>

int main(int, char **)
{
    // We create a descriptor
    WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr;

    // We create the instance using this descriptor
#ifdef WEBGPU_BACKEND_EMSCRIPTEN
    WGPUInstance instance = wgpuCreateInstance(nullptr);
#else
    WGPUInstance wgpu_instance = wgpuCreateInstance(&desc);
#endif

    // We can check whether there is actually an instance created
    if (!wgpu_instance)
    {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return 1;
    }

    std::cout << "WGPU instance: " << wgpu_instance << std::endl;

    // Adapter
    std::cout << "Requesting adapter..." << std::endl;

    WGPURequestAdapterOptions adapterOpts = {};
    adapterOpts.nextInChain = nullptr;
    WGPUAdapter adapter = requestAdapterSync(wgpu_instance, &adapterOpts);

    std::cout << "Got adapter: " << adapter << std::endl;

    //inspectAdapter(adapter);
    //inspectAdapterFeatures(adapter);
    inspectAdapterProperties(adapter);

    std::cout << "Requesting device..." << std::endl;

    WGPUDeviceDescriptor deviceDesc = {};
    {
        // A function that is invoked whenever the device stops being available.
        deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const *message, void * /* pUserData */)
        {
            std::cout << "Device lost: reason " << reason;
            if (message)
                std::cout << " (" << message << ")";
            std::cout << std::endl;
        };
    }

    WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
    std::cout << "Got device: " << device << std::endl;
    
    // We clean up the adapter immediately after getting the device
    wgpuAdapterRelease(adapter);

    // We inspect the device
    inspectDevice(device);

    // We set an error callback to catch errors that are not captured by the device.
    auto onDeviceError = [](WGPUErrorType type, char const *message, void * /* pUserData */)
    {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr /* pUserData */);

    // We create a queue
    WGPUQueue queue = wgpuDeviceGetQueue(device);

    auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status, void * /* pUserData */)
    {
        std::cout << "Queued work finished with status: " << status << std::endl;
    };
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr /* pUserData */);

    std::vector<WGPUCommandBuffer> commands;
    // [...] (Allocate and fill in command buffers)
    wgpuQueueSubmit(queue, commands.size(), commands.data());

    // Clean up the command buffers
    for (auto cmd : commands)
        wgpuCommandBufferRelease(cmd);

    // We clean up the WebGPU instance
    wgpuInstanceRelease(wgpu_instance);
    wgpuQueueRelease(queue);
    wgpuDeviceRelease(device);

    return 0;
}