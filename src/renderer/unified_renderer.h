#pragma once

#include <webgpu/webgpu.hpp>
#include <memory>
#include <vector>
#include <array>
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
    
    // Camera and transformation
    void SetViewMatrix(const float* viewMatrix);
    void SetProjectionMatrix(const float* projMatrix);
    void SetModelMatrix(const float* modelMatrix);
    
    // Viewport and settings
    void SetViewport(int width, int height);
    void SetClearColor(const Color& color);
    
    // WebGPU access for advanced usage
    wgpu::Device GetDevice() const { return m_Device; }
    wgpu::Queue GetQueue() const { return m_Queue; }

private:
    // Platform reference
    platform::IPlatform* m_Platform = nullptr;
    
    // WebGPU core objects
    wgpu::Instance m_Instance = nullptr;
    wgpu::Surface m_Surface = nullptr;
    wgpu::Device m_Device = nullptr;
    wgpu::Queue m_Queue = nullptr;
    wgpu::TextureFormat m_SurfaceFormat = wgpu::TextureFormat::Undefined;
    
    // Rendering state
    int m_Width = 0;
    int m_Height = 0;
    Color m_ClearColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
    
    // Transformation matrices
    std::array<float, 16> m_ViewMatrix;
    std::array<float, 16> m_ProjectionMatrix;
    std::array<float, 16> m_ModelMatrix;
    
    // Rendering pipelines
    wgpu::RenderPipeline m_PointPipeline = nullptr;
    wgpu::RenderPipeline m_LinePipeline = nullptr;
    wgpu::RenderPipeline m_TrianglePipeline = nullptr;
    
    // Buffers for immediate mode rendering
    wgpu::Buffer m_VertexBuffer = nullptr;
    wgpu::Buffer m_UniformBuffer = nullptr;
    wgpu::BindGroup m_UniformBindGroup = nullptr;
    
    // Vertex data for batching
    std::vector<Vertex> m_PointVertices;
    std::vector<Vertex> m_LineVertices;
    std::vector<Vertex> m_TriangleVertices;
    
    // Internal methods
    bool InitializeWebGPU();
    bool CreatePipelines();
    bool CreateBuffers();
    void UpdateUniformBuffer();
    void FlushVertexData(const std::vector<Vertex>& vertices, wgpu::RenderPipeline pipeline, wgpu::RenderPassEncoder& renderPass);
    
    // Shader creation helpers
    wgpu::ShaderModule CreateShaderModule(const char* source);
    std::string GetPointVertexShader();
    std::string GetPointFragmentShader();
    std::string GetLineVertexShader();
    std::string GetLineFragmentShader();
    std::string GetTriangleVertexShader();
    std::string GetTriangleFragmentShader();
};

} // namespace alice2
