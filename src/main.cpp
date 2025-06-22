#include <iostream>
#include <webgpu/webgpu.h>

int main (int, char**) {
	// We create a descriptor
	WGPUInstanceDescriptor desc = {};
	desc.nextInChain = nullptr;

	// We create the instance using this descriptor
	WGPUInstance instance = wgpuCreateInstance(&desc);

	// We can check whether there is actually an instance created
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
		return 1;
	}

	std::cout << "WGPU instance: " << instance << std::endl;

	// We clean up the WebGPU instance
	wgpuInstanceRelease(instance);

	return 0;
}