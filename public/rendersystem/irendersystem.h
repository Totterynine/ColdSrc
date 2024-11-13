#pragma once
#include "libcommon/module_lib.h"
#include "rendersystem/rendersystem_types.h"

class IRenderTarget
{
public:

    virtual HImage GetHardwareImage() = 0;
    virtual HImageView GetHardwareImageView() = 0;

};

class IShader;
class IDescriptorLayout;
class IDescriptorSet;

class IRenderSystem : public IModule
{
public:

    static constexpr const char *ModuleName = "RenderSystem";

    IRenderSystem() : IModule( ModuleName ) {}

    // Create the rendering system
    virtual bool Create() = 0;

    // Attach the rendering system to a window
    virtual void AttachWindow(void *window_handle, int w, int h) = 0;

    virtual IRenderTarget* CreateRenderTarget(BufferFormat fmt, int width, int height) = 0;
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
    virtual void DrawPrimitive(int first_vertex, int vertex_count) = 0;

    // Draw indexed primitives
    virtual void DrawIndexedPrimitives(int index_count) = 0;

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