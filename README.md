# zspace_alice2

A WebGPU application framework using GLFW.

## Project Structure

The project is structured to separate window management from rendering logic.

```
zspace_alice2/
|-- CMakeLists.txt
|-- depends/              # Third-party libraries (glfw, webgpu)
|-- scripts/              # Helper CMake scripts
|-- src/
|   |-- app/
|   |   |-- application.h     # Application class definition
|   |   |-- application.cpp   # Application class implementation (window, main loop)
|   |   |-- renderer.h        # Renderer class definition
|   |   |-- renderer.cpp      # Renderer class implementation (WebGPU logic)
|   |   |-- wgpu_adapter.cpp  # WebGPU adapter helper functions
|   |   `-- wgpu_device.cpp   # WebGPU device helper functions
|   `-- main.cpp            # Main application entry point
|-- README.md
`-- TODO.md
```

## Summary of Recent Changes

The initial proof-of-concept has been refactored into a more robust structure. The `Application` class now handles the GLFW window and main loop, while the `Renderer` class encapsulates all WebGPU setup and will contain all future rendering code. This separation of concerns makes the project easier to manage and extend. The build system has been updated accordingly, and several compilation issues have been resolved.

Most recently, a WebGPU surface was created and configured to draw to the GLFW window. The application now successfully renders a solid color by clearing the screen on each frame, establishing a basic render loop.
