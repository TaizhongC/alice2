#include <webgpu/webgpu.h>

struct GLFWwindow;

namespace alice2
{
    class Renderer
    {
    public:
        bool Initialize(GLFWwindow* window);
        void Terminate();

    private:
        // We put here all the variables that are shared between init and main loop
        int m_Width;
        int m_Height;
        WGPUInstance m_Instance;
        WGPUDevice m_Device;
        WGPUQueue m_Queue;
    };
} // namespace alice2