#include <webgpu/webgpu.h>

namespace alice2
{
    class Renderer
    {
    public:

    private:
        // We put here all the variables that are shared between init and main loop
        int m_Width;
        int m_Height;
        GLFWwindow *m_Window;
        Renderer *m_Renderer;

    };
} // namespace alice2