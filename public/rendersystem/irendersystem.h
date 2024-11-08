#pragma once
#include "libcommon/module_lib.h"

struct BlendState
{
    bool enable;
    float src_factor, dst_factor;
};

struct DepthStencilState
{
    bool enable;
    float depth_bias;
};

struct RasterizerState
{
    bool enable;
    int cull_mode;
};

struct Viewport
{
    int x, y, w, h;
};

struct ScissorRectangle
{
    int x, y, w, h;
};

struct ColorFloat
{
    float r, g, b, a;
};

enum class ImageFormat : short
{
    Null = 0,
    RGB8,
    RGBA8,
    RGBA16,
    RGBA16F,
    RGBA32F,
};

// Hardware image handles
using HImage = void*;
using HImageView = void*;

// Hardware shader handle
using HShader = void*;

class IVertexBuffer;
class IIndexBuffer;

class IRenderTarget
{
public:

    virtual HImage GetHardwareImage() = 0;
    virtual HImageView GetHardwareImageView() = 0;

};

enum class ShaderType : unsigned char
{
    Null = 0,
    Graphics,
    Compute,
};

enum ShaderStage : unsigned char
{
    Null = 0,
    Vertex = 0x01,
    Pixel = 0x02,
    Compute = 0x04,
};

enum class DescriptorType : short
{
    Buffer = 0,
    Image,
};

enum class PipelineBindPoint : short
{
    Graphics = 0,
    Compute
};

struct DescriptorLayoutEntry
{
    uint32_t Binding;
    DescriptorType Type;
    ShaderStage Stage;
};

class IDescriptorLayout
{
public:

};

class IDescriptorSet
{
public:
    virtual void Init(IDescriptorLayout* layout) = 0;

    virtual void BindImage(uint32_t binding, HImageView img) = 0;

    virtual void Update() = 0;
};

class IShader
{
public:

    virtual ShaderType GetType() = 0;

    virtual void SetComputeModule(HShader csModule) = 0;
    
    virtual void BuildPipeline(IDescriptorLayout* layout) = 0;

};

class IRenderSystem : public IModule
{
public:

    static constexpr const char *ModuleName = "RenderSystem";

    IRenderSystem() : IModule( ModuleName ) {}

    // Create the rendering system
    virtual bool Create() = 0;

    // Attach the rendering system to a window
    virtual void AttachWindow(void *window_handle, int w, int h) = 0;

    virtual IRenderTarget* CreateRenderTarget(ImageFormat fmt, int width, int height) = 0;
    virtual IDescriptorLayout* BuildDescriptorLayout(uint32_t numEntries, DescriptorLayoutEntry* entries) = 0;
    virtual IDescriptorSet* BuildDescriptorSet(IDescriptorLayout *layout) = 0;
    virtual IShader* CreateShader() = 0;

    virtual HShader LoadShaderModule(const char* filepath) = 0;

    // Begin rendering
    virtual void BeginRendering() = 0;

    // End rendering
    virtual void EndRendering() = 0;

    virtual void SetClearColor(ColorFloat &color) = 0;
    virtual void ClearColor() = 0;

    // Set the render target
    // Set nullptr to clear
    virtual void SetRenderTarget(IRenderTarget *target) = 0;

    // Set the viewport
    virtual void SetViewport(Viewport settings) = 0;

    // Set the scissor rectangle
    virtual void SetScissorRectangle(ScissorRectangle settings) = 0;

    // Set the current shader to render the mesh
    // Set to nullptr to clear
    virtual void BindShader(IShader *shader, PipelineBindPoint point) = 0;

    virtual void BindDescriptorSet(IDescriptorSet* set, PipelineBindPoint point) = 0;

    // Set the vertex buffer
    virtual void SetVertexBuffer(IVertexBuffer *buffer) = 0;

    // Set the index buffer
    virtual void SetIndexBuffer(IIndexBuffer *buffer) = 0;

    // Draw a primitive
    virtual void DrawPrimitive(int primitive_type, int vertex_count) = 0;

    // Draw indexed primitives
    virtual void DrawIndexedPrimitives(int primitive_type, int index_count) = 0;

    virtual void CopyRenderTargetToBackBuffer() = 0;

    // Present the render target to surface
    virtual void Present() = 0;

    virtual void Dispatch(int groupSizeX, int groupSizeY, int groupSizeZ) = 0;

    // Destroy the rendering system
    virtual void Destroy() = 0;

    // Set the blend state
    virtual void SetBlendState(BlendState settings) = 0;

    // Set the depth stencil state
    virtual void SetDepthStencilState(DepthStencilState settings) = 0;

    // Set the rasterizer state
    virtual void SetRasterizerState(RasterizerState settings) = 0;
};