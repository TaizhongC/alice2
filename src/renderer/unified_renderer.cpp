#include "unified_renderer.h"
#include "../platform/platform_interface.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <thread>
#include <chrono>

namespace alice2 {

// WGSL Shaders for WebGPU rendering
static const char* VERTEX_SHADER_SOURCE = R"(
struct Uniforms {
    mvp_matrix: mat4x4<f32>,
}

struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) color: vec4<f32>,
    @location(2) size: f32,
}

struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) size: f32,
}

@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs_main(input: VertexInput) -> VertexOutput {
    var output: VertexOutput;
    let world_pos = vec4<f32>(input.position, 1.0);
    let clip_pos = uniforms.mvp_matrix * world_pos;
    output.position = clip_pos;
    output.color = input.color;
    output.size = input.size;
    return output;
}
)";

static const char* FRAGMENT_SHADER_SOURCE = R"(
struct VertexOutput {
    @builtin(position) position: vec4<f32>,
    @location(0) color: vec4<f32>,
    @location(1) size: f32,
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
    // For now, just return the vertex color
    // Note: WebGPU doesn't have built-in point size like OpenGL
    // Point size would need to be handled differently (e.g., geometry shader or instancing)
    return input.color;
}
)";

// WebGPU callback functions
static void OnAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
    if (status == WGPURequestAdapterStatus_Success) {
        *static_cast<WGPUAdapter*>(userdata) = adapter;
    } else {
        std::cerr << "Failed to request WebGPU adapter: " << (message ? message : "Unknown error") << std::endl;
    }
}

static void OnDeviceRequestEnded(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
    if (status == WGPURequestDeviceStatus_Success) {
        *static_cast<WGPUDevice*>(userdata) = device;
    } else {
        std::cerr << "Failed to request WebGPU device: " << (message ? message : "Unknown error") << std::endl;
    }
}

static void OnDeviceError(WGPUErrorType type, char const* message, void* userdata) {
    std::cerr << "WebGPU device error (" << type << "): " << (message ? message : "Unknown error") << std::endl;
}

UnifiedRenderer::UnifiedRenderer() {
    // Initialize identity matrices
    std::fill(m_ViewMatrix.begin(), m_ViewMatrix.end(), 0.0f);
    std::fill(m_ProjectionMatrix.begin(), m_ProjectionMatrix.end(), 0.0f);
    std::fill(m_ModelMatrix.begin(), m_ModelMatrix.end(), 0.0f);
    
    // Set identity matrices
    m_ViewMatrix[0] = m_ViewMatrix[5] = m_ViewMatrix[10] = m_ViewMatrix[15] = 1.0f;
    m_ProjectionMatrix[0] = m_ProjectionMatrix[5] = m_ProjectionMatrix[10] = m_ProjectionMatrix[15] = 1.0f;
    m_ModelMatrix[0] = m_ModelMatrix[5] = m_ModelMatrix[10] = m_ModelMatrix[15] = 1.0f;
}

UnifiedRenderer::~UnifiedRenderer() {
    Shutdown();
}

bool UnifiedRenderer::Initialize(platform::IPlatform* platform) {
    m_Platform = platform;
    
    if (!InitializeWebGPU()) {
        std::cerr << "Failed to initialize WebGPU" << std::endl;
        return false;
    }
    
    if (!CreatePipelines()) {
        std::cerr << "Failed to create rendering pipelines" << std::endl;
        return false;
    }
    
    if (!CreateBuffers()) {
        std::cerr << "Failed to create buffers" << std::endl;
        return false;
    }
    
    std::cout << "UnifiedRenderer initialized successfully" << std::endl;
    return true;
}

void UnifiedRenderer::Shutdown() {
    // Clean up WebGPU resources in reverse order of creation
    if (m_UniformBindGroup) {
        wgpuBindGroupRelease(m_UniformBindGroup);
        m_UniformBindGroup = nullptr;
    }
    if (m_BindGroupLayout) {
        wgpuBindGroupLayoutRelease(m_BindGroupLayout);
        m_BindGroupLayout = nullptr;
    }
    if (m_UniformBuffer) {
        wgpuBufferRelease(m_UniformBuffer);
        m_UniformBuffer = nullptr;
    }
    if (m_VertexBuffer) {
        wgpuBufferRelease(m_VertexBuffer);
        m_VertexBuffer = nullptr;
    }
    if (m_PointPipeline) {
        wgpuRenderPipelineRelease(m_PointPipeline);
        m_PointPipeline = nullptr;
    }
    if (m_LinePipeline) {
        wgpuRenderPipelineRelease(m_LinePipeline);
        m_LinePipeline = nullptr;
    }
    if (m_TrianglePipeline) {
        wgpuRenderPipelineRelease(m_TrianglePipeline);
        m_TrianglePipeline = nullptr;
    }
    if (m_Queue) {
        wgpuQueueRelease(m_Queue);
        m_Queue = nullptr;
    }
    if (m_Device) {
        wgpuDeviceRelease(m_Device);
        m_Device = nullptr;
    }
    if (m_Surface) {
        wgpuSurfaceRelease(m_Surface);
        m_Surface = nullptr;
    }
    if (m_Instance) {
        wgpuInstanceRelease(m_Instance);
        m_Instance = nullptr;
    }

    std::cout << "UnifiedRenderer shutdown complete" << std::endl;
}

void UnifiedRenderer::BeginFrame() {
    // Clear vertex data for this frame
    m_PointVertices.clear();
    m_LineVertices.clear();
    m_TriangleVertices.clear();
}

void UnifiedRenderer::EndFrame() {
    // Update uniform buffer with current matrices
    UpdateUniformBuffer();

    // Get current surface texture
    WGPUSurfaceTexture surfaceTexture;
    wgpuSurfaceGetCurrentTexture(m_Surface, &surfaceTexture);

    if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
        std::cerr << "Failed to get current surface texture" << std::endl;
        return;
    }

    // Create texture view (use nullptr for default settings like working version)
    WGPUTextureView textureView = wgpuTextureCreateView(surfaceTexture.texture, nullptr);
    if (!textureView) {
        std::cerr << "Failed to create texture view" << std::endl;
        return;
    }

    // Create command encoder
    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "Alice2 Command Encoder";

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(m_Device, &encoderDesc);
    if (!encoder) {
        std::cerr << "Failed to create command encoder" << std::endl;
        wgpuTextureViewRelease(textureView);
        return;
    }

    // Create render pass
    WGPURenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = textureView;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = WGPULoadOp_Clear;
    colorAttachment.storeOp = WGPUStoreOp_Store;
    colorAttachment.clearValue = {m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a};

    WGPURenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;
    renderPassDesc.label = "Alice2 Render Pass";
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.depthStencilAttachment = nullptr;

    WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);
    if (!renderPass) {
        std::cerr << "Failed to begin render pass" << std::endl;
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureViewRelease(textureView);
        return;
    }

    // Set bind group for uniforms
    wgpuRenderPassEncoderSetBindGroup(renderPass, 0, m_UniformBindGroup, 0, nullptr);

    // Render points
    if (!m_PointVertices.empty()) {
        FlushVertexData(m_PointVertices, m_PointPipeline, renderPass);
    }

    // Render lines
    if (!m_LineVertices.empty()) {
        FlushVertexData(m_LineVertices, m_LinePipeline, renderPass);
    }

    // Render triangles
    if (!m_TriangleVertices.empty()) {
        FlushVertexData(m_TriangleVertices, m_TrianglePipeline, renderPass);
    }

    // End render pass
    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    // Finish command buffer
    WGPUCommandBufferDescriptor cmdBufferDesc = {};
    cmdBufferDesc.nextInChain = nullptr;
    cmdBufferDesc.label = "Alice2 Command Buffer";

    WGPUCommandBuffer commandBuffer = wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);
    if (!commandBuffer) {
        std::cerr << "Failed to finish command buffer" << std::endl;
        wgpuCommandEncoderRelease(encoder);
        wgpuTextureViewRelease(textureView);
        return;
    }

    // Submit commands
    wgpuQueueSubmit(m_Queue, 1, &commandBuffer);

    // Present surface
    wgpuSurfacePresent(m_Surface);

    // Clean up
    wgpuCommandBufferRelease(commandBuffer);
    wgpuCommandEncoderRelease(encoder);
    wgpuTextureViewRelease(textureView);
}

void UnifiedRenderer::Clear(const Color& clearColor) {
    m_ClearColor = clearColor;
}

void UnifiedRenderer::DrawPoint(const Vec3f& position, const Color& color, float size) {
    BeginPoints();
    AddPoint(position, color, size);
    EndPoints();
}

void UnifiedRenderer::DrawLine(const Vec3f& start, const Vec3f& end, const Color& color, float width) {
    BeginLines();
    AddLine(start, end, color);
    EndLines();
}

void UnifiedRenderer::DrawTriangle(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Color& color) {
    Vertex v0 = {{p0.x, p0.y, p0.z}, color, 1.0f};
    Vertex v1 = {{p1.x, p1.y, p1.z}, color, 1.0f};
    Vertex v2 = {{p2.x, p2.y, p2.z}, color, 1.0f};
    
    m_TriangleVertices.push_back(v0);
    m_TriangleVertices.push_back(v1);
    m_TriangleVertices.push_back(v2);
}

void UnifiedRenderer::BeginPoints() {
    m_PointVertices.clear();
}

void UnifiedRenderer::AddPoint(const Vec3f& position, const Color& color, float size) {
    Vertex vertex = {{position.x, position.y, position.z}, color, size};
    m_PointVertices.push_back(vertex);
}

void UnifiedRenderer::EndPoints() {
    // Points will be rendered in EndFrame()
}

void UnifiedRenderer::BeginLines() {
    m_LineVertices.clear();
}

void UnifiedRenderer::AddLine(const Vec3f& start, const Vec3f& end, const Color& color) {
    Vertex v0 = {{start.x, start.y, start.z}, color, 1.0f};
    Vertex v1 = {{end.x, end.y, end.z}, color, 1.0f};
    
    m_LineVertices.push_back(v0);
    m_LineVertices.push_back(v1);
}

void UnifiedRenderer::EndLines() {
    // Lines will be rendered in EndFrame()
}

void UnifiedRenderer::BeginTriangles() {
    m_TriangleVertices.clear();
}

void UnifiedRenderer::AddTriangle(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Color& color) {
    Vertex v0 = {{p0.x, p0.y, p0.z}, color, 1.0f};
    Vertex v1 = {{p1.x, p1.y, p1.z}, color, 1.0f};
    Vertex v2 = {{p2.x, p2.y, p2.z}, color, 1.0f};

    m_TriangleVertices.push_back(v0);
    m_TriangleVertices.push_back(v1);
    m_TriangleVertices.push_back(v2);
}

void UnifiedRenderer::EndTriangles() {
    // Triangles will be rendered in EndFrame()
}

void UnifiedRenderer::SetViewMatrix(const float* viewMatrix) {
    if (viewMatrix) {
        std::copy(viewMatrix, viewMatrix + 16, m_ViewMatrix.begin());
    }
}

void UnifiedRenderer::SetProjectionMatrix(const float* projMatrix) {
    if (projMatrix) {
        std::copy(projMatrix, projMatrix + 16, m_ProjectionMatrix.begin());
    }
}

void UnifiedRenderer::SetModelMatrix(const float* modelMatrix) {
    if (modelMatrix) {
        std::copy(modelMatrix, modelMatrix + 16, m_ModelMatrix.begin());
    }
}

void UnifiedRenderer::SetViewport(int width, int height) {
    m_Width = width;
    m_Height = height;
    std::cout << "Setting viewport to " << width << "x" << height << std::endl;

    // TODO: Reconfigure WebGPU surface when implemented
}

void UnifiedRenderer::SetClearColor(const Color& color) {
    m_ClearColor = color;
}



bool UnifiedRenderer::InitializeWebGPU() {
    std::cout << "Initializing WebGPU with C API..." << std::endl;

    // Get viewport size
    auto [width, height] = m_Platform->GetFramebufferSize();
    m_Width = width;
    m_Height = height;
    std::cout << "Viewport size: " << m_Width << "x" << m_Height << std::endl;

    // 1. Create WebGPU instance
    WGPUInstanceDescriptor instanceDesc = {};
    instanceDesc.nextInChain = nullptr;
    m_Instance = wgpuCreateInstance(&instanceDesc);
    if (!m_Instance) {
        std::cerr << "Failed to create WebGPU instance" << std::endl;
        return false;
    }
    std::cout << "✓ WebGPU instance created" << std::endl;

    // 2. Create surface from platform
    m_Surface = static_cast<WGPUSurface>(m_Platform->CreateWebGPUSurface(m_Instance));
    if (!m_Surface) {
        std::cerr << "Failed to create WebGPU surface" << std::endl;
        return false;
    }
    std::cout << "✓ WebGPU surface created" << std::endl;

    // 3. Request adapter
    WGPURequestAdapterOptions adapterOptions = {};
    adapterOptions.nextInChain = nullptr;
    adapterOptions.compatibleSurface = m_Surface;
    adapterOptions.powerPreference = WGPUPowerPreference_HighPerformance;
    adapterOptions.backendType = WGPUBackendType_Undefined;
    adapterOptions.forceFallbackAdapter = false;

    WGPUAdapter adapter = nullptr;
    wgpuInstanceRequestAdapter(m_Instance, &adapterOptions, OnAdapterRequestEnded, &adapter);

    // Wait for adapter request to complete (simple polling)
    int attempts = 0;
    while (!adapter && attempts < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        attempts++;
    }

    if (!adapter) {
        std::cerr << "Failed to get WebGPU adapter" << std::endl;
        return false;
    }
    std::cout << "✓ WebGPU adapter obtained" << std::endl;

    // 4. Request device
    WGPUDeviceDescriptor deviceDesc = {};
    deviceDesc.nextInChain = nullptr;
    deviceDesc.label = "Alice2 WebGPU Device";
    deviceDesc.requiredFeatureCount = 0;
    deviceDesc.requiredFeatures = nullptr;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "Alice2 Default Queue";

    wgpuAdapterRequestDevice(adapter, &deviceDesc, OnDeviceRequestEnded, &m_Device);

    // Wait for device request to complete
    attempts = 0;
    while (!m_Device && attempts < 100) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        attempts++;
    }

    if (!m_Device) {
        std::cerr << "Failed to get WebGPU device" << std::endl;
        return false;
    }
    std::cout << "✓ WebGPU device obtained" << std::endl;

    // Set up error callback
    wgpuDeviceSetUncapturedErrorCallback(m_Device, OnDeviceError, nullptr);

    // Get the queue
    m_Queue = wgpuDeviceGetQueue(m_Device);
    if (!m_Queue) {
        std::cerr << "Failed to get WebGPU queue" << std::endl;
        return false;
    }
    std::cout << "✓ WebGPU queue obtained" << std::endl;

    // 5. Configure surface (use preferred format like working version)
    m_SurfaceFormat = wgpuSurfaceGetPreferredFormat(m_Surface, adapter);

    std::cout << "Surface format: " << m_SurfaceFormat << std::endl;

    WGPUSurfaceConfiguration surfaceConfig = {};
    surfaceConfig.nextInChain = nullptr;
    surfaceConfig.device = m_Device;
    surfaceConfig.format = m_SurfaceFormat;
    surfaceConfig.usage = WGPUTextureUsage_RenderAttachment;
    surfaceConfig.width = m_Width;
    surfaceConfig.height = m_Height;
    surfaceConfig.presentMode = WGPUPresentMode_Fifo;
    surfaceConfig.alphaMode = WGPUCompositeAlphaMode_Auto;
    surfaceConfig.viewFormatCount = 0;
    surfaceConfig.viewFormats = nullptr;

    wgpuSurfaceConfigure(m_Surface, &surfaceConfig);
    std::cout << "✓ WebGPU surface configured" << std::endl;

    // Clean up adapter (no longer needed)
    wgpuAdapterRelease(adapter);

    std::cout << "WebGPU initialization complete!" << std::endl;
    return true;
}

bool UnifiedRenderer::CreatePipelines() {
    std::cout << "Creating WebGPU rendering pipelines..." << std::endl;

    // Create shader modules
    WGPUShaderModule vertexShader = CreateShaderModule(VERTEX_SHADER_SOURCE);
    WGPUShaderModule fragmentShader = CreateShaderModule(FRAGMENT_SHADER_SOURCE);

    if (!vertexShader || !fragmentShader) {
        std::cerr << "Failed to create shader modules" << std::endl;
        return false;
    }
    std::cout << "✓ Shader modules created" << std::endl;

    // Create bind group layout for uniforms
    WGPUBindGroupLayoutEntry bindGroupLayoutEntry = {};
    bindGroupLayoutEntry.binding = 0;
    bindGroupLayoutEntry.visibility = WGPUShaderStage_Vertex;
    bindGroupLayoutEntry.buffer.type = WGPUBufferBindingType_Uniform;
    bindGroupLayoutEntry.buffer.hasDynamicOffset = false;
    bindGroupLayoutEntry.buffer.minBindingSize = sizeof(float) * 16; // 4x4 matrix

    WGPUBindGroupLayoutDescriptor bindGroupLayoutDesc = {};
    bindGroupLayoutDesc.nextInChain = nullptr;
    bindGroupLayoutDesc.label = "Alice2 Uniform Bind Group Layout";
    bindGroupLayoutDesc.entryCount = 1;
    bindGroupLayoutDesc.entries = &bindGroupLayoutEntry;

    WGPUBindGroupLayout bindGroupLayout = wgpuDeviceCreateBindGroupLayout(m_Device, &bindGroupLayoutDesc);
    if (!bindGroupLayout) {
        std::cerr << "Failed to create bind group layout" << std::endl;
        return false;
    }
    std::cout << "✓ Bind group layout created" << std::endl;

    // Create pipeline layout
    WGPUPipelineLayoutDescriptor pipelineLayoutDesc = {};
    pipelineLayoutDesc.nextInChain = nullptr;
    pipelineLayoutDesc.label = "Alice2 Pipeline Layout";
    pipelineLayoutDesc.bindGroupLayoutCount = 1;
    pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;

    WGPUPipelineLayout pipelineLayout = wgpuDeviceCreatePipelineLayout(m_Device, &pipelineLayoutDesc);
    if (!pipelineLayout) {
        std::cerr << "Failed to create pipeline layout" << std::endl;
        return false;
    }
    std::cout << "✓ Pipeline layout created" << std::endl;

    // Define vertex attributes
    WGPUVertexAttribute vertexAttributes[3] = {};

    // Position attribute (location 0)
    vertexAttributes[0].format = WGPUVertexFormat_Float32x3;
    vertexAttributes[0].offset = 0;
    vertexAttributes[0].shaderLocation = 0;

    // Color attribute (location 1)
    vertexAttributes[1].format = WGPUVertexFormat_Float32x4;
    vertexAttributes[1].offset = sizeof(float) * 3;
    vertexAttributes[1].shaderLocation = 1;

    // Size attribute (location 2)
    vertexAttributes[2].format = WGPUVertexFormat_Float32;
    vertexAttributes[2].offset = sizeof(float) * 7;
    vertexAttributes[2].shaderLocation = 2;

    // Define vertex buffer layout
    WGPUVertexBufferLayout vertexBufferLayout = {};
    vertexBufferLayout.arrayStride = sizeof(Vertex);
    vertexBufferLayout.stepMode = WGPUVertexStepMode_Vertex;
    vertexBufferLayout.attributeCount = 3;
    vertexBufferLayout.attributes = vertexAttributes;

    // Create point pipeline
    WGPURenderPipelineDescriptor pointPipelineDesc = {};
    pointPipelineDesc.nextInChain = nullptr;
    pointPipelineDesc.label = "Alice2 Point Pipeline";
    pointPipelineDesc.layout = pipelineLayout;

    pointPipelineDesc.vertex.module = vertexShader;
    pointPipelineDesc.vertex.entryPoint = "vs_main";
    pointPipelineDesc.vertex.bufferCount = 1;
    pointPipelineDesc.vertex.buffers = &vertexBufferLayout;

    WGPUFragmentState fragmentState = {};
    fragmentState.module = fragmentShader;
    fragmentState.entryPoint = "fs_main";

    WGPUColorTargetState colorTarget = {};
    colorTarget.format = m_SurfaceFormat;
    colorTarget.blend = nullptr; // No blending for now
    colorTarget.writeMask = WGPUColorWriteMask_All;

    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pointPipelineDesc.fragment = &fragmentState;

    pointPipelineDesc.primitive.topology = WGPUPrimitiveTopology_PointList;
    pointPipelineDesc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pointPipelineDesc.primitive.frontFace = WGPUFrontFace_CCW;
    pointPipelineDesc.primitive.cullMode = WGPUCullMode_None;

    pointPipelineDesc.multisample.count = 1;
    pointPipelineDesc.multisample.mask = ~0u;
    pointPipelineDesc.multisample.alphaToCoverageEnabled = false;

    m_PointPipeline = wgpuDeviceCreateRenderPipeline(m_Device, &pointPipelineDesc);
    if (!m_PointPipeline) {
        std::cerr << "Failed to create point pipeline" << std::endl;
        return false;
    }
    std::cout << "✓ Point pipeline created" << std::endl;

    // Create line pipeline (same as point but different topology)
    WGPURenderPipelineDescriptor linePipelineDesc = pointPipelineDesc;
    linePipelineDesc.label = "Alice2 Line Pipeline";
    linePipelineDesc.primitive.topology = WGPUPrimitiveTopology_LineList;

    m_LinePipeline = wgpuDeviceCreateRenderPipeline(m_Device, &linePipelineDesc);
    if (!m_LinePipeline) {
        std::cerr << "Failed to create line pipeline" << std::endl;
        return false;
    }
    std::cout << "✓ Line pipeline created" << std::endl;

    // Create triangle pipeline (same as point but different topology)
    WGPURenderPipelineDescriptor trianglePipelineDesc = pointPipelineDesc;
    trianglePipelineDesc.label = "Alice2 Triangle Pipeline";
    trianglePipelineDesc.primitive.topology = WGPUPrimitiveTopology_TriangleList;

    m_TrianglePipeline = wgpuDeviceCreateRenderPipeline(m_Device, &trianglePipelineDesc);
    if (!m_TrianglePipeline) {
        std::cerr << "Failed to create triangle pipeline" << std::endl;
        return false;
    }
    std::cout << "✓ Triangle pipeline created" << std::endl;

    // Store bind group layout for buffer creation
    m_BindGroupLayout = bindGroupLayout;

    // Clean up shader modules (no longer needed)
    wgpuShaderModuleRelease(vertexShader);
    wgpuShaderModuleRelease(fragmentShader);
    wgpuPipelineLayoutRelease(pipelineLayout);

    std::cout << "Pipeline creation complete!" << std::endl;
    return true;
}

bool UnifiedRenderer::CreateBuffers() {
    std::cout << "Creating WebGPU buffers..." << std::endl;

    // Initialize transformation matrices to identity
    std::fill(m_ViewMatrix.begin(), m_ViewMatrix.end(), 0.0f);
    std::fill(m_ProjectionMatrix.begin(), m_ProjectionMatrix.end(), 0.0f);
    std::fill(m_ModelMatrix.begin(), m_ModelMatrix.end(), 0.0f);

    // Set identity matrices
    m_ViewMatrix[0] = m_ViewMatrix[5] = m_ViewMatrix[10] = m_ViewMatrix[15] = 1.0f;
    m_ProjectionMatrix[0] = m_ProjectionMatrix[5] = m_ProjectionMatrix[10] = m_ProjectionMatrix[15] = 1.0f;
    m_ModelMatrix[0] = m_ModelMatrix[5] = m_ModelMatrix[10] = m_ModelMatrix[15] = 1.0f;

    // Create vertex buffer (dynamic, will be updated each frame)
    WGPUBufferDescriptor vertexBufferDesc = {};
    vertexBufferDesc.nextInChain = nullptr;
    vertexBufferDesc.label = "Alice2 Vertex Buffer";
    vertexBufferDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    vertexBufferDesc.size = sizeof(Vertex) * 1000; // Allocate space for up to 1000 vertices
    vertexBufferDesc.mappedAtCreation = false;

    m_VertexBuffer = wgpuDeviceCreateBuffer(m_Device, &vertexBufferDesc);
    if (!m_VertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        return false;
    }
    std::cout << "✓ Vertex buffer created" << std::endl;

    // Create uniform buffer for MVP matrix
    WGPUBufferDescriptor uniformBufferDesc = {};
    uniformBufferDesc.nextInChain = nullptr;
    uniformBufferDesc.label = "Alice2 Uniform Buffer";
    uniformBufferDesc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    uniformBufferDesc.size = sizeof(float) * 16; // 4x4 matrix
    uniformBufferDesc.mappedAtCreation = false;

    m_UniformBuffer = wgpuDeviceCreateBuffer(m_Device, &uniformBufferDesc);
    if (!m_UniformBuffer) {
        std::cerr << "Failed to create uniform buffer" << std::endl;
        return false;
    }
    std::cout << "✓ Uniform buffer created" << std::endl;

    // Create bind group for uniforms
    WGPUBindGroupEntry bindGroupEntry = {};
    bindGroupEntry.binding = 0;
    bindGroupEntry.buffer = m_UniformBuffer;
    bindGroupEntry.offset = 0;
    bindGroupEntry.size = sizeof(float) * 16;

    WGPUBindGroupDescriptor bindGroupDesc = {};
    bindGroupDesc.nextInChain = nullptr;
    bindGroupDesc.label = "Alice2 Uniform Bind Group";
    bindGroupDesc.layout = m_BindGroupLayout;
    bindGroupDesc.entryCount = 1;
    bindGroupDesc.entries = &bindGroupEntry;

    m_UniformBindGroup = wgpuDeviceCreateBindGroup(m_Device, &bindGroupDesc);
    if (!m_UniformBindGroup) {
        std::cerr << "Failed to create uniform bind group" << std::endl;
        return false;
    }
    std::cout << "✓ Uniform bind group created" << std::endl;

    // Upload initial identity matrix to uniform buffer
    UpdateUniformBuffer();

    std::cout << "Buffer creation complete!" << std::endl;
    return true;
}

void UnifiedRenderer::UpdateUniformBuffer() {
    // Combine view and projection matrices
    std::array<float, 16> viewProjectionMatrix;

    // Matrix multiplication: viewProjection = projection * view
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += m_ProjectionMatrix[row * 4 + k] * m_ViewMatrix[k * 4 + col];
            }
            viewProjectionMatrix[row * 4 + col] = sum;
        }
    }

    // Debug: Print matrix values occasionally
    static int debugCounter = 0;
    if (debugCounter % 300 == 0) {
        std::cout << "View matrix: [" << m_ViewMatrix[0] << "," << m_ViewMatrix[1] << "," << m_ViewMatrix[2] << "," << m_ViewMatrix[3] << "]" << std::endl;
        std::cout << "Proj matrix: [" << m_ProjectionMatrix[0] << "," << m_ProjectionMatrix[1] << "," << m_ProjectionMatrix[2] << "," << m_ProjectionMatrix[3] << "]" << std::endl;
        std::cout << "MVP matrix: [" << viewProjectionMatrix[0] << "," << viewProjectionMatrix[1] << "," << viewProjectionMatrix[2] << "," << viewProjectionMatrix[3] << "]" << std::endl;

        // Print full MVP matrix for debugging
        std::cout << "Full MVP matrix:" << std::endl;
        for (int i = 0; i < 4; i++) {
            std::cout << "[" << viewProjectionMatrix[i*4+0] << ", " << viewProjectionMatrix[i*4+1] << ", "
                      << viewProjectionMatrix[i*4+2] << ", " << viewProjectionMatrix[i*4+3] << "]" << std::endl;
        }
    }
    debugCounter++;

    // Upload to WebGPU uniform buffer
    wgpuQueueWriteBuffer(m_Queue, m_UniformBuffer, 0, viewProjectionMatrix.data(), sizeof(float) * 16);
}

void UnifiedRenderer::FlushVertexData(const std::vector<Vertex>& vertices, WGPURenderPipeline pipeline, WGPURenderPassEncoder renderPass) {
    if (vertices.empty() || !pipeline || !renderPass) {
        return;
    }

    // Debug: Print first vertex data occasionally
    static int debugCounter = 0;
    if (debugCounter % 300 == 0 && !vertices.empty()) {
        const auto& v = vertices[0];
        std::cout << "First vertex: pos(" << v.position.x << "," << v.position.y << "," << v.position.z
                  << ") color(" << v.color.r << "," << v.color.g << "," << v.color.b << "," << v.color.a
                  << ") size(" << v.size << ")" << std::endl;
    }
    debugCounter++;

    // Upload vertex data to buffer
    size_t dataSize = vertices.size() * sizeof(Vertex);
    wgpuQueueWriteBuffer(m_Queue, m_VertexBuffer, 0, vertices.data(), dataSize);

    // Set pipeline
    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);

    // Set vertex buffer
    wgpuRenderPassEncoderSetVertexBuffer(renderPass, 0, m_VertexBuffer, 0, dataSize);

    // Draw vertices
    wgpuRenderPassEncoderDraw(renderPass, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    std::cout << "Rendered " << vertices.size() << " vertices" << std::endl;
}

WGPUShaderModule UnifiedRenderer::CreateShaderModule(const char* source) {
    WGPUShaderModuleWGSLDescriptor wgslDesc = {};
    wgslDesc.chain.next = nullptr;
    wgslDesc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
    wgslDesc.code = source;

    WGPUShaderModuleDescriptor shaderDesc = {};
    shaderDesc.nextInChain = &wgslDesc.chain;
    shaderDesc.label = "Alice2 Shader Module";

    WGPUShaderModule shaderModule = wgpuDeviceCreateShaderModule(m_Device, &shaderDesc);
    if (!shaderModule) {
        std::cerr << "Failed to create shader module" << std::endl;
        return nullptr;
    }

    return shaderModule;
}

} // namespace alice2
