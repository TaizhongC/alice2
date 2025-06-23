#pragma once

#include <webgpu/webgpu.h>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include "../core/base/Types.h"

// Forward declarations
namespace alice2 { namespace platform { class IPlatform; } }

namespace alice2 {

// Vertex structure for immediate mode rendering
struct Vertex {
    Vec3f position;
    Color color;
    float size; // For points
};

class UnifiedRenderer {
public:
    UnifiedRenderer();
    ~UnifiedRenderer();
    
    // Core lifecycle
    bool Initialize(platform::IPlatform* platform);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    
    // Immediate mode rendering API (CODA-compatible)
    void Clear(const Color& clearColor = Color(0.2f, 0.2f, 0.2f, 1.0f));
    void DrawPoint(const Vec3f& position, const Color& color = Color::Red(), float size = 5.0f);
    void DrawLine(const Vec3f& start, const Vec3f& end, const Color& color = Color::Black(), float width = 1.0f);
    void DrawTriangle(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Color& color = Color::White());
    
    // Batch rendering for performance
    void BeginPoints();
    void AddPoint(const Vec3f& position, const Color& color, float size = 5.0f);
    void EndPoints();
    
    void BeginLines();
    void AddLine(const Vec3f& start, const Vec3f& end, const Color& color);
    void EndLines();

    void BeginTriangles();
    void AddTriangle(const Vec3f& p0, const Vec3f& p1, const Vec3f& p2, const Color& color);
    void EndTriangles();
    
    // Camera and transformation
    void SetViewMatrix(const float* viewMatrix);
    void SetProjectionMatrix(const float* projMatrix);
    void SetModelMatrix(const float* modelMatrix);
    
    // Viewport and settings
    void SetViewport(int width, int height);
    void SetClearColor(const Color& color);
    
    // WebGPU access for advanced usage
    WGPUDevice GetDevice() const { return m_Device; }
    WGPUQueue GetQueue() const { return m_Queue; }

private:
    // Platform reference
    platform::IPlatform* m_Platform = nullptr;

    // WebGPU core objects
    WGPUInstance m_Instance = nullptr;
    WGPUSurface m_Surface = nullptr;
    WGPUDevice m_Device = nullptr;
    WGPUQueue m_Queue = nullptr;
    WGPUTextureFormat m_SurfaceFormat = WGPUTextureFormat_Undefined;
    
    // Rendering state
    int m_Width = 0;
    int m_Height = 0;
    Color m_ClearColor = Color(0.0f, 0.0f, 0.0f, 1.0f); // Black background for better line visibility
    
    // Transformation matrices
    std::array<float, 16> m_ViewMatrix;
    std::array<float, 16> m_ProjectionMatrix;
    std::array<float, 16> m_ModelMatrix;
    
    // Rendering pipelines
    WGPURenderPipeline m_PointPipeline = nullptr;
    WGPURenderPipeline m_LinePipeline = nullptr;
    WGPURenderPipeline m_TrianglePipeline = nullptr;

    // Buffers for immediate mode rendering
    WGPUBuffer m_VertexBuffer = nullptr;
    WGPUBuffer m_UniformBuffer = nullptr;
    WGPUBindGroup m_UniformBindGroup = nullptr;
    WGPUBindGroupLayout m_BindGroupLayout = nullptr;
    
    // Vertex data for batching
    std::vector<Vertex> m_PointVertices;
    std::vector<Vertex> m_LineVertices;
    std::vector<Vertex> m_TriangleVertices;
    
    // Internal methods
    bool InitializeWebGPU();
    bool CreatePipelines();
    bool CreateBuffers();
    void UpdateUniformBuffer();
    void FlushVertexData(const std::vector<Vertex>& vertices, WGPURenderPipeline pipeline, WGPURenderPassEncoder renderPass);

    // Shader creation helpers
    WGPUShaderModule CreateShaderModule(const char* source);
};

} // namespace alice2
