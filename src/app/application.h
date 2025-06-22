#include <glfw/glfw3.h>

namespace alice2
{
    class Application
    {
    public:
        // Initialize everything and return true if it went all right
        bool Initialize(int width, int height);

        // Uninitialize everything that was initialized
        void Terminate();

        // Draw a frame and handle events
        void MainLoop();

        // Return true as long as the main loop should keep on running
        bool IsRunning();

    private:
        // We put here all the variables that are shared between init and main loop
        int m_Width;
        int m_Height;
        GLFWwindow *m_Window;
        Renderer *m_Renderer;

    };
} // namespace alice2